import re
import os
import subprocess
import sys

main_dir = os.path.dirname(os.path.realpath(__file__)) + '/..'
asset_dir = main_dir + '/asset'
example_dir = main_dir + '/examples'
openabl_bin = main_dir + '/OpenABL'

if not os.path.isfile(openabl_bin):
    print("OpenABL binary not found at " + openabl_bin)
    sys.exit(1)

def openabl_run(model, backend, params, config):
    model_file = example_dir + '/' + model + '.abl'
    args = [
        openabl_bin,
        '-i', model_file,
        '-b', backend,
        '-A', asset_dir,
        '-R'
    ]

    for key, value in params.items():
        args.append('-P')
        args.append(key + '=' + str(value))

    for key, value in config.items():
        args.append('-C')
        args.append(key + '=' + str(value))

    return subprocess.check_output(args, stderr=subprocess.STDOUT)

def openabl_get_exec_time(model, backend, params, config):
    output = openabl_run(model, backend, params, config)
    matches = re.search('Execution time: (.*)s', output)
    if matches is None:
        raise RuntimeError('Failed to extract execution time')
    return float(matches.group(1))

def next_pow2(n):
    return 2**(n-1).bit_length()

if len(sys.argv) < 3:
    print('Usage: bench.py backend model')
    sys.exit(1)

backend = sys.argv[1]
model = sys.argv[2]

num_timesteps = 100
min_num_agents = 250
max_num_agents = 200000
num_agents_factor = 2

print('n,t')
num_agents = min_num_agents
while num_agents <= max_num_agents:
    params = {
        'num_timesteps': num_timesteps,
        'num_agents': num_agents,
    }

    # TODO Find a better solution for this
    config = {}
    if backend == 'flamegpu':
        config['flamegpu.buffer_size'] = next_pow2(num_agents)

    exec_time = openabl_get_exec_time(model, backend, params, config)
    print(str(num_agents) + ',' + str(exec_time))

    num_agents *= num_agents_factor
