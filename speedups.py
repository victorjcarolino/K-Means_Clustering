import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV files
df_s = pd.read_csv('speedup_to_s.csv')
df_bs = pd.read_csv('speedup_to_bs.csv')

# Plot the data
plt.figure(figsize=(10, 6))

# Plot speedup_to_s.csv data
plt.plot(df_s['Datafile'], df_s['Speedup'], label='Speedup to Serial')

# Plot speedup_to_bs.csv data
plt.plot(df_bs['Datafile'], df_bs['Speedup'], label='Speedup to Better-Serial')

# Add labels and title
plt.xlabel('Datafile')
plt.ylabel('Speedup')
plt.title('Speedup Comparison')
plt.legend()

# Save the plot as a PNG file
plt.savefig('speedup_comparison.png')

# Show the plot
plt.show()