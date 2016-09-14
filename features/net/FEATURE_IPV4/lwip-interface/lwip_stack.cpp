/* LWIP implementation of NetworkInterfaceAPI
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nsapi.h"
#include "mbed_interface.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "eth_arch.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/tcp.h"
#include "lwip/ip.h"
#include "netif/etharp.h"
#include "netif/ppp_oe.h"
#include "EmacInterface.h"

EmacInterface *mac;
static err_t default_etharp_output(struct netif *netif, struct pbuf *q, ip_addr_t *ipaddr);
static err_t low_level_output(struct netif *netif, struct pbuf *p);
static err_t eth_enetif_init(struct netif *netif);

/* Static arena of sockets */
static struct lwip_socket {
    bool in_use;

    struct netconn *conn;
    struct netbuf *buf;
    u16_t offset;

    void (*cb)(void *);
    void *data;
} lwip_arena[MEMP_NUM_NETCONN];

static void lwip_arena_init(void)
{
    memset(lwip_arena, 0, sizeof lwip_arena);
}

static struct lwip_socket *lwip_arena_alloc(void)
{
    sys_prot_t prot = sys_arch_protect();

    for (int i = 0; i < MEMP_NUM_NETCONN; i++) {
        if (!lwip_arena[i].in_use) {
            struct lwip_socket *s = &lwip_arena[i];
            memset(s, 0, sizeof *s);
            s->in_use = true;
            sys_arch_unprotect(prot);
            return s;
        }
    }

    sys_arch_unprotect(prot);
    return 0;
}

static void lwip_arena_dealloc(struct lwip_socket *s)
{
    s->in_use = false;
}

static void lwip_socket_callback(struct netconn *nc, enum netconn_evt eh, u16_t len)
{
    sys_prot_t prot = sys_arch_protect();

    for (int i = 0; i < MEMP_NUM_NETCONN; i++) {
        if (lwip_arena[i].in_use
            && lwip_arena[i].conn == nc
            && lwip_arena[i].cb) {
            lwip_arena[i].cb(lwip_arena[i].data);
        }
    }

    sys_arch_unprotect(prot);
}


/* TCP/IP and Network Interface Initialisation */
static struct netif lwip_netif;

static char lwip_ip_addr[NSAPI_IP_SIZE] = "\0";
static char lwip_mac_addr[NSAPI_MAC_SIZE] = "\0";

static sys_sem_t lwip_tcpip_inited;
static void lwip_tcpip_init_irq(void *eh)
{
    sys_sem_signal(&lwip_tcpip_inited);
}

static sys_sem_t lwip_netif_linked;
static void lwip_netif_link_irq(struct netif *lwip_netif)
{
    if (netif_is_link_up(lwip_netif)) {
        sys_sem_signal(&lwip_netif_linked);
    }
}

static sys_sem_t lwip_netif_up;
static void lwip_netif_status_irq(struct netif *lwip_netif)
{
    if (netif_is_up(lwip_netif)) {
        strcpy(lwip_ip_addr, inet_ntoa(lwip_netif->ip_addr));
        sys_sem_signal(&lwip_netif_up);
    }
}

static void lwip_set_mac_address(void)
{
#if (MBED_MAC_ADDRESS_SUM != MBED_MAC_ADDR_INTERFACE)
    snprintf(lwip_mac_addr, 19, "%02x:%02x:%02x:%02x:%02x:%02x",
            MBED_MAC_ADDR_0, MBED_MAC_ADDR_1, MBED_MAC_ADDR_2,
            MBED_MAC_ADDR_3, MBED_MAC_ADDR_4, MBED_MAC_ADDR_5);
#else
    char mac[6];
    mbed_mac_address(mac);
    snprintf(lwip_mac_addr, 19, "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
}


/* LWIP interface implementation */
const char *lwip_get_mac_address(void)
{
    return lwip_mac_addr[0] ? lwip_mac_addr : 0;
}

const char *lwip_get_ip_address(void)
{
    return lwip_ip_addr[0] ? lwip_ip_addr : 0;
}

int lwip_bringup(void)
{
    // Check if we've already connected
    if (!lwip_get_mac_address()) {
        // Set up network
        lwip_set_mac_address();

        sys_sem_new(&lwip_tcpip_inited, 0);
        sys_sem_new(&lwip_netif_linked, 0);
        sys_sem_new(&lwip_netif_up, 0);

        tcpip_init(lwip_tcpip_init_irq, NULL);
        sys_arch_sem_wait(&lwip_tcpip_inited, 0);

        memset(&lwip_netif, 0, sizeof lwip_netif);
        netif_add(&lwip_netif, 0, 0, 0, NULL, eth_enetif_init, tcpip_input);
        netif_set_default(&lwip_netif);

        netif_set_link_callback  (&lwip_netif, lwip_netif_link_irq);
        netif_set_status_callback(&lwip_netif, lwip_netif_status_irq);

        //eth_arch_enable_interrupts();
    }

    // Zero out socket set
    lwip_arena_init();

    // Connect to the network
    dhcp_start(&lwip_netif);

    // Wait for an IP Address
    u32_t ret = sys_arch_sem_wait(&lwip_netif_up, 15000);
    if (ret == SYS_ARCH_TIMEOUT) {
        return NSAPI_ERROR_DHCP_FAILURE;
    }

    return 0;
}

void lwip_bringdown(void)
{
    // Disconnect from the network
    dhcp_release(&lwip_netif);
    dhcp_stop(&lwip_netif);
    lwip_ip_addr[0] = '\0';
}


/* LWIP error remapping */
static int lwip_err_remap(err_t err) {
    switch (err) {
        case ERR_OK:
            return 0;
        case ERR_MEM:
            return NSAPI_ERROR_NO_MEMORY;
        case ERR_CONN:
        case ERR_CLSD:
            return NSAPI_ERROR_NO_CONNECTION;
        case ERR_TIMEOUT:
        case ERR_RTE:
        case ERR_INPROGRESS:
        case ERR_WOULDBLOCK:
            return NSAPI_ERROR_WOULD_BLOCK;
        case ERR_VAL:
        case ERR_USE:
        case ERR_ISCONN:
        case ERR_ARG:
            return NSAPI_ERROR_PARAMETER;
        default:
            return NSAPI_ERROR_DEVICE_ERROR;
    }
}


/* LWIP network stack implementation */
static nsapi_addr_t lwip_getaddr(nsapi_stack_t *stack)
{
    if (!lwip_get_ip_address()) {
        nsapi_addr_t zero_addr = {};
        return zero_addr;
    }

    nsapi_addr_t addr;
    addr.version = NSAPI_IPv4;
    inet_aton(lwip_get_ip_address(), (ip_addr_t *)addr.bytes);
    return addr;
}

static int lwip_gethostbyname(nsapi_stack_t *stack, nsapi_addr_t *addr, const char *host)
{
    err_t err = netconn_gethostbyname(host, (ip_addr_t *)addr->bytes);
    if (err != ERR_OK) {
        return NSAPI_ERROR_DNS_FAILURE;
    }

    addr->version = NSAPI_IPv4;
    return 0;
}

static int lwip_socket_open(nsapi_stack_t *stack, nsapi_socket_t *handle, nsapi_protocol_t proto)
{
    struct lwip_socket *s = lwip_arena_alloc();
    if (!s) {
        return NSAPI_ERROR_NO_SOCKET;
    }

    s->conn = netconn_new_with_callback(
            proto == NSAPI_TCP ? NETCONN_TCP : NETCONN_UDP,
            lwip_socket_callback);

    if (!s->conn) {
        lwip_arena_dealloc(s);
        return NSAPI_ERROR_NO_SOCKET;
    }

    netconn_set_recvtimeout(s->conn, 1);
    *(struct lwip_socket **)handle = s;
    return 0;
}

static int lwip_socket_close(nsapi_stack_t *stack, nsapi_socket_t handle)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;

    err_t err = netconn_delete(s->conn);
    lwip_arena_dealloc(s);
    return lwip_err_remap(err);
}


static int lwip_socket_bind(nsapi_stack_t *stack, nsapi_socket_t handle, nsapi_addr_t addr, uint16_t port)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;
    if (addr.version != NSAPI_IPv4) {
        return NSAPI_ERROR_PARAMETER;
    }

    err_t err = netconn_bind(s->conn, (ip_addr_t *)addr.bytes, port);
    return lwip_err_remap(err);
}

static int lwip_socket_listen(nsapi_stack_t *stack, nsapi_socket_t handle, int backlog)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;

    err_t err = netconn_listen_with_backlog(s->conn, backlog);
    return lwip_err_remap(err);
}

static int lwip_socket_connect(nsapi_stack_t *stack, nsapi_socket_t handle, nsapi_addr_t addr, uint16_t port)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;
    if (addr.version != NSAPI_IPv4) {
        return NSAPI_ERROR_PARAMETER;
    }

    netconn_set_nonblocking(s->conn, false);
    err_t err = netconn_connect(s->conn, (ip_addr_t *)addr.bytes, port);
    netconn_set_nonblocking(s->conn, true);

    return lwip_err_remap(err);
}

static int lwip_socket_accept(nsapi_stack_t *stack, nsapi_socket_t server, nsapi_socket_t *handle, nsapi_addr_t *addr, uint16_t *port)
{
    struct lwip_socket *s = (struct lwip_socket *)server;
    struct lwip_socket *ns = lwip_arena_alloc();

    err_t err = netconn_accept(s->conn, &ns->conn);
    if (err != ERR_OK) {
        lwip_arena_dealloc(ns);
        return lwip_err_remap(err);
    }

    netconn_set_recvtimeout(ns->conn, 1);
    *(struct lwip_socket **)handle = ns;

    (void) netconn_peer(ns->conn, (ip_addr_t *)addr->bytes, port);
    addr->version = NSAPI_IPv4;

    return 0;
}

static int lwip_socket_send(nsapi_stack_t *stack, nsapi_socket_t handle, const void *data, unsigned size)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;

    err_t err = netconn_write(s->conn, data, size, NETCONN_COPY);
    if (err != ERR_OK) {
        return lwip_err_remap(err);
    }

    return size;
}

static int lwip_socket_recv(nsapi_stack_t *stack, nsapi_socket_t handle, void *data, unsigned size)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;

    if (!s->buf) {
        err_t err = netconn_recv(s->conn, &s->buf);
        s->offset = 0;

        if (err != ERR_OK) {
            return (err == ERR_CLSD) ? 0 : lwip_err_remap(err);
        }
    }

    u16_t recv = netbuf_copy_partial(s->buf, data, (u16_t)size, s->offset);
    s->offset += recv;

    if (s->offset >= netbuf_len(s->buf)) {
        netbuf_delete(s->buf);
        s->buf = 0;
    }

    return recv;
}

static int lwip_socket_sendto(nsapi_stack_t *stack, nsapi_socket_t handle, nsapi_addr_t addr, uint16_t port, const void *data, unsigned size)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;
    if (addr.version != NSAPI_IPv4) {
        return NSAPI_ERROR_PARAMETER;
    }

    struct netbuf *buf = netbuf_new();
    err_t err = netbuf_ref(buf, data, (u16_t)size);
    if (err != ERR_OK) {
        netbuf_free(buf);
        return lwip_err_remap(err);;
    }

    err = netconn_sendto(s->conn, buf, (ip_addr_t *)addr.bytes, port);
    netbuf_delete(buf);
    if (err != ERR_OK) {
        return lwip_err_remap(err);
    }

    return size;
}

static int lwip_socket_recvfrom(nsapi_stack_t *stack, nsapi_socket_t handle, nsapi_addr_t *addr, uint16_t *port, void *data, unsigned size)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;

    struct netbuf *buf;
    err_t err = netconn_recv(s->conn, &buf);
    if (err != ERR_OK) {
        return lwip_err_remap(err);
    }

    addr->version = NSAPI_IPv4;
    memcpy(addr->bytes, netbuf_fromaddr(buf), sizeof addr->bytes);
    *port = netbuf_fromport(buf);

    u16_t recv = netbuf_copy(buf, data, (u16_t)size);
    netbuf_delete(buf);

    return recv;
}

static int lwip_setsockopt(nsapi_stack_t *stack, nsapi_socket_t handle, int level, int optname, const void *optval, unsigned optlen)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;

    switch (optname) {
        case NSAPI_KEEPALIVE:
            if (optlen != sizeof(int) || s->conn->type != NETCONN_TCP) {
                return NSAPI_ERROR_UNSUPPORTED;
            }

            s->conn->pcb.tcp->so_options |= SOF_KEEPALIVE;
            return 0;

        case NSAPI_KEEPIDLE:
            if (optlen != sizeof(int) || s->conn->type != NETCONN_TCP) {
                return NSAPI_ERROR_UNSUPPORTED;
            }

            s->conn->pcb.tcp->keep_idle = *(int*)optval;
            return 0;

        case NSAPI_KEEPINTVL:
            if (optlen != sizeof(int) || s->conn->type != NETCONN_TCP) {
                return NSAPI_ERROR_UNSUPPORTED;
            }

            s->conn->pcb.tcp->keep_intvl = *(int*)optval;
            return 0;

        case NSAPI_REUSEADDR:
            if (optlen != sizeof(int)) {
                return NSAPI_ERROR_UNSUPPORTED;
            }

            if (*(int *)optval) {
                s->conn->pcb.tcp->so_options |= SOF_REUSEADDR;
            } else {
                s->conn->pcb.tcp->so_options &= ~SOF_REUSEADDR;
            }
            return 0;

        default:
            return NSAPI_ERROR_UNSUPPORTED;
    }
}

static void lwip_socket_attach(nsapi_stack_t *stack, nsapi_socket_t handle, void (*callback)(void *), void *data)
{
    struct lwip_socket *s = (struct lwip_socket *)handle;

    s->cb = callback;
    s->data = data;
}

class LwipMemAllocator : public StackMemory
{
public:
    LwipMemAllocator() {
        // Nothing to do
    }

    ~LwipMemAllocator() {
        // Nothing to do
    }

    virtual StackMem *alloc(uint32_t size, uint32_t align) {
        struct pbuf *pbuf;
        //TODO - check for power of 2 and for null
        pbuf = pbuf_alloc(PBUF_RAW, size + align, PBUF_RAM);
        if (pbuf != NULL) {
            uint32_t remainder = (uint32_t)pbuf->payload % align;
            uint32_t offset = align - remainder;
            if (offset >= align) {
                offset = align;
            }
            pbuf->payload = (void*)((char*)pbuf->payload + offset);
            pbuf->tot_len -= offset;
            pbuf->len -= offset;
        }
        return (StackMem *)pbuf;
    }

    virtual void free(StackMem *ptr) {
        pbuf_free((struct pbuf *)ptr);
    }

    virtual uint8_t* data_ptr(StackMem *ptr) {
        return (uint8_t*)((struct pbuf *)ptr)->payload;
    }

    virtual uint32_t len(StackMem *ptr) {
        return ((struct pbuf *)ptr)->len;
    }
    virtual void set_len(StackMem *ptr, uint32_t len) {
        struct pbuf *p = (struct pbuf *)ptr;
        p->len = len;
        //TODO - assert length only decreases
    }
    virtual StackMem *dequeue_alloc(StackMemChain **ptr) {
        struct pbuf **link = (struct pbuf **)ptr;
        struct pbuf *p = *link;
        *link = (*link)->next;
        return (StackMem *)p;

    }
    virtual void enqueue_free(StackMemChain *ptr, StackMem *mem) {
        //TODO
    }
    virtual uint32_t len(StackMemChain* ptr) {
        return ((struct pbuf *)ptr)->tot_len;
    }
};


static int add_mac(nsapi_stack_t *stack, EmacInterface *new_mac)
{
    static LwipMemAllocator allocator;
    new_mac->set_mem_allocator(&allocator);
    // Only one interface can be added at the moment
    MBED_ASSERT(NULL == mac);
    mac = new_mac;
    return 0;
}


/* LWIP network stack */
const nsapi_stack_api_t lwip_stack_api = {
    /* .get_ip_address      */ lwip_getaddr,
    /* .gethostbyname       */ lwip_gethostbyname,
    /* .setstackopt         */ NULL,
    /* .getstackopt         */ NULL,
    /* .socket_open         */ lwip_socket_open,
    /* .socket_close        */ lwip_socket_close,
    /* .socket_bind         */ lwip_socket_bind,
    /* .socket_listen       */ lwip_socket_listen,
    /* .socket_connect      */ lwip_socket_connect,
    /* .socket_accept       */ lwip_socket_accept,
    /* .socket_send         */ lwip_socket_send,
    /* .socket_recv         */ lwip_socket_recv,
    /* .socket_sendto       */ lwip_socket_sendto,
    /* .socket_recvfrom     */ lwip_socket_recvfrom,
    /* .socket_attach       */ lwip_socket_attach,
    /* .setsockopt          */ lwip_setsockopt,
    /* .getsockopt          */ NULL,
    /* .add_mac             */ add_mac,
};

nsapi_stack_t lwip_stack = {
    .stack_api = &lwip_stack_api,
};

/**
 * This function is the ethernet packet send function. It calls
 * etharp_output after checking link status.
 *
 * \param[in] netif the lwip network interface structure for this enetif
 * \param[in] q Pointer to pbug to send
 * \param[in] ipaddr IP address
 * \return ERR_OK or error code
 */
static err_t default_etharp_output(struct netif *netif, struct pbuf *q, ip_addr_t *ipaddr)
{
  /* Only send packet is link is up */
  if (netif->flags & NETIF_FLAG_LINK_UP)
    return etharp_output(netif, q, ipaddr);

  return ERR_CONN;
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    bool ret = mac->linkoutput((StackMemChain*)p);
    return ret ? ERR_OK : ERR_IF;
}

/** \brief  Attempt to read a packet from the EMAC interface.
 *
 *  \param[in] netif the lwip network interface structure
 *  \param[in] idx   index of packet to be read
 */
void enetif_input(void *user_data, StackMem* buf)
{
  struct eth_hdr *ethhdr;
  struct pbuf *p = (struct pbuf *)buf;
  struct netif *netif = (struct netif *)user_data;

  /* points to packet payload, which starts with an Ethernet header */
  ethhdr = (struct eth_hdr*)p->payload;

  switch (htons(ethhdr->type)) {
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
#if PPPOE_SUPPORT
    case ETHTYPE_PPPOEDISC:
    case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
      /* full packet send to tcpip_thread to process */
      if (netif->input(p, netif) != ERR_OK) {
        LWIP_DEBUGF(NETIF_DEBUG, ("k64f_enetif_input: IP input error\n"));
        /* Free buffer */
        pbuf_free(p);
      }
      break;

    default:
      /* Return buffer */
      pbuf_free(p);
      break;
  }
}

static void link_state_change(void *user_data, bool up)
{
    struct netif *netif = (struct netif *)user_data;
    if (up) {
        tcpip_callback_with_block((tcpip_callback_fn)netif_set_link_up, (void*) netif, 1);
    } else {
        tcpip_callback_with_block((tcpip_callback_fn)netif_set_link_down, (void*) netif, 1);
    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param[in] netif the lwip network interface structure for this netif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
static err_t eth_enetif_init(struct netif *netif)
{
  mac->set_link_input(enetif_input, (void*)netif);
  mac->set_link_state_cb(link_state_change, (void*)netif);

  /* set MAC hardware address */
  mac->get_hwaddr(netif->hwaddr);
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* maximum transfer unit */
  netif->mtu = mac->get_mtu_size();

  /* device capabilities */
  // TODOETH: check if the flags are correct below
  // TODO: Get flags from mac
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP;

  netif->hostname = (char*)mac->get_hostname();
  netif->name[0] = 'e';
  netif->name[1] = 'n';

  netif->output = default_etharp_output;
  netif->linkoutput = low_level_output;

  mac->set_hwaddr(netif->hwaddr);

  mac->powerup();

  return ERR_OK;
}

