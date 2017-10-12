import re
import os
import subprocess
import sys

main_dir = os.path.dirname(os.path.realpath(__file__)) + '/..'
asset_dir = main_dir + '/asset'
example_dir = main_dir + '/examples'
openabl_bin = main_dir + '/OpenABL'

result_dir = os.environ.get('RESULT_DIR')
if result_dir is not None:
    if not os.path.isdir(result_dir):
        print('Result directory ' + result_dir + ' does not exist')
        sys.exit(1)
else:
    print('WARNING: No RESULT_DIR specified')

if not os.path.isfile(openabl_bin):
    print('OpenABL binary not found at ' + openabl_bin)
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

    try:
        return subprocess.check_output(args, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as err:
        print('Invocation of command\n' + ' '.join(args) + '\n'
                + 'exited with exit code ' + str(err.returncode)
                + ' and the following output:\n' + err.output)
        sys.exit(1)

def openabl_get_exec_time(model, backend, params, config):
    output = openabl_run(model, backend, params, config)
    matches = re.search('Execution time: (.*)s', output)
    if matches is None:
        raise RuntimeError('Failed to extract execution time')
    return float(matches.group(1))

def next_pow2(n):
    return 2**(n-1).bit_length()

def run_bench(backend, model, (min_num_agents, max_num_agents)):
    num_timesteps = 100
    num_agents_factor = 2

    result = "n,t\n"
    print('Running ' + model + ' on ' + backend)

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
        csv_str = str(num_agents) + ',' + str(exec_time)

        result += csv_str + "\n"
        print(csv_str)

        num_agents *= num_agents_factor

    if result_dir is not None:
        file_name = result_dir + '/bench_' + model + '_' + backend + '.txt'
        with open(file_name, 'w') as f:
            f.write(result)

if len(sys.argv) < 3:
    print('Usage: [RESULT_DIR=data/] bench.py backend model [min_agents-max_agents]')
    sys.exit(1)

backend = sys.argv[1]
if backend == 'flamegpu' and 'SMS' not in os.environ:
    print('Using flamegpu backend without SMS environment variable')
    sys.exit(1)

models = sys.argv[2].split(',')

num_agents_range = (250, 128000)
if len(sys.argv) >= 4:
    num_agents_spec = sys.argv[3].split('-')
    if len(num_agents_spec) != 2:
        print('Invalid argent number specification (min-max)')
        sys.exit(1)

    num_agents_range = (int(num_agents_spec[0]), int(num_agents_spec[1]))

for model in models:
    run_bench(backend, model, num_agents_range)
