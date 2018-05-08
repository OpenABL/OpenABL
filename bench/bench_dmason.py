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

import argparse
import os
import sys
import openabl

default_models = [
    'circle', 'boids2d_flockers', 'game_of_life',
    'sugarscape', 'ants', 'predator_prey'
]

parser = argparse.ArgumentParser()
parser.add_argument('-t', '--num-threads', type=int, required=True,
    help='Number of threads')
parser.add_argument('-m', '--models',
    help='Models to benchmark (comma separated)')
parser.add_argument('-n', '--num-agents', type=int, default="100000",
    help='Number of agents')
parser.add_argument('-T', '--num-timesteps', type=int, default="100",
    help='Number of timesteps')
parser.add_argument('-r', '--result-dir',
    help='Directory for benchmark results')
args = parser.parse_args()

num_timesteps = args.num_timesteps
num_agents = args.num_agents

result_dir = args.result_dir
if result_dir is not None:
    if not os.path.isdir(result_dir):
        os.makedirs(result_dir)
else:
    print('WARNING: No result directory specified')

try:
    runner = openabl.create_auto()
except openabl.OpenAblNotFound as err:
    print(str(err))
    sys.exit(1)

max_threads = args.num_threads
if max_threads & (max_threads - 1) != 0:
    print('Number of threads must be power of two')
    sys.exit(1)

if args.models:
    models = args.models.split(',')
else:
    models = default_models

for model in models:
    num_rows = 1
    num_cols = 2
    inc_rows = True

    result = "n,t\n"
    print('Running %s with 2-%d threads' % (model, max_threads))
    while num_rows * num_cols <= max_threads:
        num_threads = num_rows * num_cols

        params = {
            'num_timesteps': num_timesteps,
            'num_agents': num_agents,
        }

        config = {
            'dmason.grid_rows': num_rows,
            'dmason.grid_cols': num_cols,
        }

        # The default parametrization of the predator_prey model works
        # really badly for D-Mason
        if model == 'predator_prey':
            params['SPACE_MULT'] = 128

        try:
            exec_time = runner.get_exec_time(model, 'dmason', params, config)
        except openabl.InvocationFailed as err:
            print(str(err))
            continue

        csv_str = str(num_threads) + ',' + str(exec_time)
        result += csv_str + "\n"
        print(csv_str)

        if inc_rows:
            num_rows *= 2
            inc_rows = False
        else:
            num_cols *= 2
            inc_rows = True

    if result_dir is not None:
        file_name = result_dir + '/scale_' + model + '.txt'
        with open(file_name, 'w') as f:
            f.write(result)
