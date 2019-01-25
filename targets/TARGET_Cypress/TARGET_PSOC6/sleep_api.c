/*
 * mbed Microcontroller Library
 * Copyright (c) 2017-2018 Future Electronics
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

#include "cmsis.h"
#include "device.h"
#include "cy_syspm.h"

#ifdef DEVICE_SLEEP

void hal_sleep(void)
{
    Cy_SysPm_Sleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
}

void hal_deepsleep(void)
{
#if DEVICE_LPTICKER

    if (CY_SYSPM_SUCCESS == Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT))
    {
        /* Restore clocks after the wakeup from Deep Sleep */

        uint32_t fllpll; /* 0 = FLL, all other values = PLL */

        for (fllpll = 0UL; fllpll <= CY_SRSS_NUM_PLL; fllpll++)
        {
            /* If FLL or PLL is enabled */
            if (0UL != ((fllpll == 0UL) ? (_FLD2VAL(SRSS_CLK_FLL_CONFIG_FLL_ENABLE, SRSS_CLK_FLL_CONFIG)) : 
                                          (_FLD2VAL(SRSS_CLK_PLL_CONFIG_ENABLE, SRSS_CLK_PLL_CONFIG[fllpll - 1UL]))))
            {
                uint32_t timeoutus;

                /* Timeout wait for FLL or PLL to regain lock */
                for (timeoutus = 1000; ((fllpll == 0UL) ? Cy_SysClk_FllLocked() : Cy_SysClk_PllLocked(fllpll)) && (0UL != timeoutus); timeoutus--)
                {
                    Cy_SysLib_DelayUs(1U);
                }
                
                if (0UL != timeoutus)
                {
                    /* Set the FLL/PLL bypass mode to OUTPUT */
                    if(fllpll == 0UL)
                    {
                        CY_REG32_CLR_SET(SRSS_CLK_FLL_CONFIG3, SRSS_CLK_FLL_CONFIG3_BYPASS_SEL, (uint32_t)CY_SYSCLK_FLLPLL_OUTPUT_OUTPUT);
                    }
                    else
                    {
                        CY_REG32_CLR_SET(SRSS_CLK_PLL_CONFIG[fllpll - 1UL], SRSS_CLK_PLL_CONFIG_BYPASS_SEL, (uint32_t)CY_SYSCLK_FLLPLL_OUTPUT_OUTPUT);
                    }
                }
                else
                {
                    /* Clock were not restored */
                }
            }
        }
    }

#endif /* DEVICE_LPTICKER */
}

#endif // DEVICE_SLEEP
