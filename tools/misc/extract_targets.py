#!/usr/bin/env python
"""
 mbed
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

import sys
import os
import json
import argparse
from os.path import join, abspath, dirname
from flash_algo import PackFlashAlgo

# Be sure that the tools directory is in the search path
ROOT = abspath(join(dirname(__file__), "..", ".."))
sys.path.insert(0, ROOT)

from tools.targets import TARGETS
from tools.arm_pack_manager import Cache

def main():
    """Regenerate all flash algorithms"""
    parser = argparse.ArgumentParser(description='Flash generator')
    parser.add_argument("--rebuild_cache", action="store_true",
                        help="Rebuild entire cache")
    args = parser.parse_args()

    cache = Cache(True, True)
    if args.rebuild_cache:
        cache.cache_everything()
        print("Cache rebuilt")
        return

    if not os.path.exists("output"):
        os.mkdir("output")

    count_target = 0
    count_supported = 0
    for target in TARGETS:
        device_name = None
        count_target += 1
        if hasattr(target, "device_name"):
            device_name = target.device_name
            print("Device: %s" % device_name)
            try:
                algo_binary = cache.get_flash_algorthim_binary(device_name)
            except:
                algo_binary = None

            if algo_binary is not None:
                count_supported += 1
                device_name = device_name.replace("/", "-")
                algo = PackFlashAlgo(algo_binary.read())

                template_path = "mbed_flash_api_code.tmpl"
                output_path = join("output", device_name + ".c")
                algo.process_template(template_path, output_path)
        print("%s: %s" % (target.name, device_name))
    print("%s of %s supported" % (count_supported, count_target))

if __name__ == '__main__':
    main()
