# Copyright 2018 OpenABL Contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import re
import os
import subprocess

class InvocationFailed(Exception):
    pass

class OpenAblNotFound(Exception):
    pass

class OpenAbl:
    def __init__(self, openabl_bin, example_dir, asset_dir):
        self.openabl_bin = openabl_bin
        self.example_dir = example_dir
        self.asset_dir = asset_dir

    def run(self, model, backend, params, config):
        model_file = self.example_dir + '/' + model + '.abl'
        args = [
            self.openabl_bin,
            '-i', model_file,
            '-b', backend,
            '-A', self.asset_dir,
            '-R'
        ]

        for key, value in params.items():
            args.append('-P')
            args.append(key + '=' + str(value))

        for key, value in config.items():
            args.append('-C')
            args.append(key + '=' + str(value))

        try:
            return subprocess.check_output(args, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as err:
            raise InvocationFailed(
                'Invocation of command\n' + ' '.join(args) + '\n'
                    + 'exited with exit code ' + str(err.returncode)
                    + ' and the following output:\n' + err.output.decode('utf-8'))

    def get_exec_time(self, model, backend, params, config):
        output = self.run(model, backend, params, config).decode('utf-8')
        matches = re.search('Execution time: (.*)s', output)
        if matches is None:
            raise RuntimeError('Failed to extract execution time')
        return float(matches.group(1))

        openabl_bin = None
        for try_openabl_bin in try_openabl_bins:
            if os.path.isfile(try_openabl_bin):
                openabl_bin = try_openabl_bin
                break

def create_auto():
    main_dir = os.path.dirname(os.path.realpath(__file__)) + '/..'
    try_openabl_bins = [
        main_dir + '/OpenABL',
        main_dir + '/build/OpenABL'
    ]

    openabl_bin = None
    for try_openabl_bin in try_openabl_bins:
        if os.path.isfile(try_openabl_bin):
            openabl_bin = try_openabl_bin
            break

    if openabl_bin is None:
        raise OpenAblNotFound(
            'OpenABL binary not found. Tried: ' + \
                ', '.join(try_openabl_bins))

    example_dir = main_dir + '/examples'
    asset_dir = main_dir + '/asset'
    return OpenAbl(openabl_bin, example_dir, asset_dir)
