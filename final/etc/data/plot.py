#!/usr/bin/env python

import seaborn as sns
import pandas as pd
import matplotlib.pyplot as plt


def main():
    """
    Main plotting code
    """
    sns.set(style="whitegrid")

    data_frame = pd.read_csv("simple-data.csv")
    g = sns.barplot(x="program", y="time", hue="simulated", data=data_frame)
    g.set_xlabel("Test Program")
    g.set_ylabel("Time taken (in s)")
    plt.savefig("plot.png")


if __name__ == "__main__":
    main()
