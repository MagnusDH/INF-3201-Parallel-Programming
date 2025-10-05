import numpy
import matplotlib.pyplot as plt

nodes = [4, 5, 6, 7, 8, 9, 10]

sequential = 0.098

parallel = numpy.array([0.336, 0.263, 0.281, 0.261, 0.286, 0.297, 0.296])
openMP = numpy.array([0.318, 0.405, 0.423, 0.370, 0.505, 0.482, 0.468])


# Create the plot
plt.figure(figsize=(8, 5))
plt.plot(nodes, parallel, 'o-', color='green', label='Parallel', linewidth=2, markersize=6)
plt.plot(nodes, openMP, 'o-', color='red', label='Parallel with openMP', linewidth=2, markersize=6)

# Add the sequential baseline as a horizontal line
plt.axhline(y=sequential, color='blue', linestyle='-', linewidth=2, label='Sequential')

# Labels and formatting
plt.title('Execution Time vs Number of Processes')
plt.xlabel('Number of Processes')
plt.ylabel('Execution Time (seconds)')
plt.legend()
plt.grid(True, linestyle='--', alpha=0.6)

# Show the plot
plt.tight_layout()
plt.show()
