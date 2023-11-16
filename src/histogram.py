import matplotlib.pyplot as plt

# Initialize lists to store data
latency = []
count = []

# Read data from the file
with open('histogram.out', 'r') as file:
    lines = file.readlines()
    # skip the headers and all and start reading (so skip 4 lines)
    for line in lines[4:]:
        parts = line.split()
        latency_str = parts[0].rstrip('+')  # Remove '+' sign if present
        latency.append(latency_str)
        count.append(int(parts[1]))

# Create the bar plot
plt.figure(figsize=(10, 6))
plt.bar([i + 2 for i in range(len(latency))], count, label='counts', color='b', alpha=0.7, align='center', tick_label=latency)
plt.xlabel('Latency(cycles)')
plt.ylabel('Count')
plt.title('Address Count by Latency')
plt.legend()
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.xticks(rotation=90)
plt.tight_layout()
plt.show()
plt.savefig('histogram.pdf')
