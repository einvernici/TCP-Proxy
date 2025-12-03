import pandas as pd
import matplotlib.pyplot as plt

# Load your logfile
df = pd.read_csv("metrics.csv")

# Sort by time just in case
df = df.sort_values("timestamp_ms")

# Metrics present in your log (excluding source and timestamp)
metrics = ["rtt", "rttvar", "cwnd", "ssthresh", "retrans", "lost"]

# Generate one chart per metric
for metric in metrics:
    if metric in df.columns:
        plt.figure()
        plt.plot(df["timestamp_ms"], df[metric])
        plt.xlabel("Time (ms)")
        plt.ylabel(metric)
        plt.title(f"{metric} over time")
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f"{metric}_over_time.png")
        plt.close()

print("Charts generated!")
