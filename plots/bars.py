import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
matplotlib.rcParams['font.family'] = 'serif'



def generate(values, labels, name):
    fig, ax = plt.subplots()

    bars = ax.bar(
        x=np.arange(len(values)),
        height=values,
        tick_label=labels,
        color = ['#539caf', '#7663b0', '#66c2a5']
    )

    # Axis formatting.
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_visible(False)
    ax.spines['bottom'].set_color('#DDDDDD')
    ax.tick_params(bottom=False, left=False)
    ax.set_axisbelow(True)
    ax.yaxis.grid(True, color='#EEEEEE')
    ax.xaxis.grid(False)
    ax.set_ylim(0,1)
    # Add text annotations to the top of the bars.


    ax.set_xlabel('Parsing function', labelpad=15, color='#333333')
    ax.set_ylabel('GB/s', labelpad=15, color='#333333')


    fig.tight_layout()
    fig.savefig(name,    # Set path and filename
                dpi = 300,                     # Set dots per inch
                bbox_inches="tight",           # Remove extra whitespace around plot
                facecolor='white')             # Set background color to white


values = [0.82, 0.3, 0.05]
labels = ['simdzone', 'Knot DNS 3.3.4', 'NSD 4.9']
generate(values, labels, 'bars_com.pdf')
values = [0.86, 0.19, 0.07]
generate(values, labels, 'bars_se.pdf')

