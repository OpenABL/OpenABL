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

import csv
import math
import re
import os
import sys
import matplotlib.pyplot as plt


# Matches order in Euro-Par paper
model_order = [
    'circle', 'boids2d_flockers', 'game_of_life',
    'sugarscape', 'ants', 'predator_prey',
]

if len(sys.argv) < 2:
    print('Usage: python plot_dmason.py result_dir/')
    sys.exit(1)

result_dir = sys.argv[1]
if not os.path.isdir(result_dir):
    print('Result directory ' + result_dir + ' does not exist')
    sys.exit(1)

# returns ([num_threads], [times])
def read_data_from_file(file_name):
    with open(file_name, 'r') as f:
        reader = csv.reader(f)
        # skip first line
        next(reader)

        num_agents = []
        times = []
        for row in reader:
            if len(row) != 2:
                print('Skipping malformed row')
                continue

            num_agents.append(float(row[0]))
            times.append(float(row[1]))

        return (num_agents, times)

def ordered_items(dictionary, key_order):
    dict_copy = dictionary.copy()
    for key in key_order:
        if key in dict_copy:
            yield key, dict_copy[key]
            del dict_copy[key]
    for key, value in dict_copy.items():
        yield key, value

def get_ideal(times):
    time = times[0]
    ideal = []
    for _ in times:
        ideal.append(time)
        time /= 2
    return ideal

file_name_re = re.compile('^scale_(.+)\.txt$')

# {model: ([num_threads], [times])}
data = {}
for file_name in os.listdir(result_dir):
    match = file_name_re.match(file_name)
    if match is None:
        print('Skipping ' + file_name)
        continue

    model = match.group(1)
    path_name = result_dir + '/' + file_name

    data[model] = read_data_from_file(path_name)

num_models = len(data)
num_cols = 3
num_rows = int(math.ceil(num_models / num_cols))
fig, axes = plt.subplots(num_rows, num_cols)

for ax_row in axes:
    ax_row[0].set_ylabel('Runtime (s)')

axes = axes.reshape(-1)
i = 0
for model, (num_threads, times) in ordered_items(data, model_order):
    ax = axes[i]
    ax.plot(num_threads, times, label='DMason')
    ax.plot(num_threads, get_ideal(times), label='ideal')
    ax.set_xlabel(model)
    ax.set_yscale('log')
    ax.set_xscale('log', basex=2)
    ax.set_xticks(num_threads)
    ax.legend(loc='upper right', prop={'size': 8})
    i += 1

fig.set_size_inches(8, 6)
plt.tight_layout()
plt.show()
