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

def openabl_run(model, backend, params):
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

    return subprocess.check_output(args)

def openabl_get_exec_time(model, backend, params):
    output = openabl_run(model, backend, params)
    matches = re.search('Execution time: (.*)s', output)
    if matches is None:
        raise RuntimeError('Failed to extract execution time')
    return float(matches.group(1))

model = 'circle'
backend = 'mason'
num_timesteps = 100
min_num_agents = 1000
max_num_agents = 200000
num_agents_factor = 2

print('n,t')
num_agents = min_num_agents
while num_agents <= max_num_agents:
    params = {
        'num_timesteps': num_timesteps,
        'num_agents': num_agents,
    }

    exec_time = openabl_get_exec_time(model, backend, params)
    print(str(num_agents) + ',' + str(exec_time))

    num_agents *= num_agents_factor
