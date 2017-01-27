"""
mbed SDK
Copyright (c) 2017-2017 ARM Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

TEST_BLOB = """main_bootloader,0x10000
main,*
nonvol, 0x1000
"""


def main():
    
    rom_start = 1024
    rom_size = 1024 * 1024
    regions = layout_to_regions(TEST_BLOB, rom_start, rom_size)
    defines = regions_to_defines(regions)
    #print("Regions: %s" % (regions,))
    print("Defines: %s" % (defines,))


def get_layout_problems(layout_blob):
    #TODO
    return None


def layout_to_regions(layout_data, rom_start, rom_size):
    """Extract the regions from a layout file

    Each region is a dictionary containing "name", "addr" and "size".

    Positional arguments:
    layout_data - contents of layout.txt
    rom_start - start of the target's rom block
    rom_size - size of the target's rom block
    """
    region_list = []
    for line in layout_data.splitlines():
        name, size_str = [entry.strip() for entry in line.split(",")]
        size = _size_str_to_value(size_str)
        region = {
            "name": name,
            "size": size,
        }
        region_list.append(region)
    return _regions_to_abs_regions(region_list, rom_start, rom_size)


def regions_to_defines(regions):
    """Get a list of defines for state and size of all regions

    Positional arguments:
    regions - a list of regions each of with have a name, addr and size
    """
    define_list = []
    for region in regions:
        name = region["name"].upper()
        define_list.append("-D%s_ADDR=0x%x" % (name, region["addr"]))
        define_list.append("-D%s_SIZE=0x%x" % (name, region["size"]))
    return define_list


def regions_to_common_pairs(regions):
    """Return name, value pairs to be used as defines in the 'common' group

    Positional arguments:
    regions - a list of regions each of with have a name, addr and size
    """
    define_list = []
    for region in regions:
        name = region["name"].upper()
        define_list.append(("%s_ADDR" % name, region["addr"]))
        define_list.append(("%s_SIZE" % name, region["size"]))
    return define_list


def regions_to_ld_pairs(regions, entry, rom_start):
    """Return name, value pairs to be used as defines in the 'ld' group

    Positional arguments:
    regions - a list of regions each of with have a name, addr and size
    entry - name of the application entry point
    rom_start - address of the starting block of rom
    """
    rgn_list = [rgn for rgn in regions if rgn["name"] == entry]
    assert len(rgn_list) == 1, "Expected 1 region, found %s" % len(rgn_list)
    region = rgn_list[0]
    return (
        ("MBED_APP_OFFSET", region["addr"] - rom_start),
        ("MBED_APP_SIZE", region["size"])
    )


def _get_wildcard_size(regions, rom_size):
    """Get the size of the wildcard region"""
    allocated_size = sum(region["size"] for region in regions if
                         region["size"] is not None)
    return rom_size - allocated_size


def _regions_to_abs_regions(regions, rom_start, rom_size):
    """Take a list of regions with just size and compute absoute address"""
    wildcard_size = _get_wildcard_size(regions, rom_size)
    abs_regions = []
    offset = rom_start
    for region in regions:
        size = region["size"] if region["size"] is not None else wildcard_size
        region = {
            "name": region["name"],
            "addr": offset,
            "size": size,
        }
        offset += size
        abs_regions.append(region)
    return abs_regions


def _size_str_to_value(size_str):
    """Convert a size string to a value or None if wildcard"""
    if size_str == "*":
        return None
    if size_str[0:2] == "0x":
        return int(size_str[2:], 16)
    else:
        return int(size_str)


if __name__ == '__main__':
    main()

