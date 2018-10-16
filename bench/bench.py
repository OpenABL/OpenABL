# Copyright 2017 OpenABL Contributors
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
import time
import openabl

default_backends = [
    'c', 'mason', 'flame', 'flamegpu',
]

default_models = [
    'circle', 'boids2d', 'game_of_life',
    'sugarscape', 'ants', 'predator_prey'
]

default_agent_ranges = {
    'c': (250, 32000),
    'mason': (250, 128000),
    'mason2': (250, 128000),
    'dmason': (250, 128000),
    'flame': (250, 4000),
    'flamegpu': (250, 1024000),
}

epilog = '''
Default models:
    * circle
    * boids2d
    * game_of_life
    * sugarscape
    * ants
    * predator_prey

Default agent ranges:
    * c:        250-32000
    * mason:    250-128000
    * mason2:   250-128000
    * flame:    250-4000
    * flamegpu: 250-10240000
'''

parser = argparse.ArgumentParser(
    epilog=epilog,
    formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument('-b', '--backends',
    help='Backends to benchmark (comma separated)')
parser.add_argument('-m', '--models',
    help='Models to benchmark (comma separated)')
parser.add_argument('-n', '--num-agents',
    help='Number of agent range (min-max)')
parser.add_argument('-r', '--result-dir',
    help='Directory for benchmark results')
parser.add_argument('-M', '--max-time',
    metavar='SEC', type=int,
    help='(Apprimate) maximal time per backend per model')
args = parser.parse_args()

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

def next_pow2(n):
    return 2**(n-1).bit_length()

def run_bench(runner, backend, model, num_agents_range, max_time):
    (min_num_agents, max_num_agents) = num_agents_range
    num_timesteps = 100
    num_agents_factor = 2

    result = "n,t\n"
    print('Running %s on %s backend with %d-%d agents' %
        (model, backend, min_num_agents, max_num_agents))

    start_time = time.time()
    num_agents = min_num_agents
    while num_agents <= max_num_agents:
        cur_start_time = time.time()

        params = {
            'num_timesteps': num_timesteps,
            'num_agents': num_agents,
        }

        # TODO Find a better solution for this
        config = {}
        if backend == 'flamegpu':
            config['flamegpu.buffer_size'] = next_pow2(num_agents)

        try:
            exec_time = runner.get_exec_time(model, backend, params, config)
        except openabl.InvocationFailed as err:
            print(str(err))
            return

        csv_str = str(num_agents) + ',' + str(exec_time)

        result += csv_str + "\n"
        print(csv_str)

        num_agents *= num_agents_factor

        total_time = time.time() - start_time
        cur_time = time.time() - cur_start_time

        # Add time of current run as an estimate for the next
        if max_time is not None and total_time + cur_time > max_time:
            break


    if result_dir is not None:
        file_name = result_dir + '/bench_' + model + '_' + backend + '.txt'
        with open(file_name, 'w') as f:
            f.write(result)

if args.backends:
    backends = args.backends.split(',')
else:
    backends = default_backends

if 'flamegpu' in backends and 'SMS' not in os.environ:
    print('When using flamegpu the SM architecture must be specified using the SMS environment variable (e.g. SMS=52)')
    sys.exit(1)

if args.models:
    models = args.models.split(',')
else:
    models = default_models

for backend in backends:
    for model in models:
        if args.num_agents:
            num_agents_spec = args.num_agents.split('-')
            if len(num_agents_spec) != 2:
                print('Invalid agent number specification (min-max)')
                sys.exit(1)

            num_agents_range = (int(num_agents_spec[0]), int(num_agents_spec[1]))
        else:
            num_agents_range = default_agent_ranges[backend]

        run_bench(runner, backend, model, num_agents_range, args.max_time)
