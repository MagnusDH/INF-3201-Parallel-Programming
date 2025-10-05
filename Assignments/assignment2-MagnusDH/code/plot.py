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



"""
sequential: 0.098588 seconds

parallel 10 processes: 0.283691, 0.306463, 0.298345 =  0.296
parallel 9 processes: 0.312813, 0.266012, 0.312669 = 0.2974
parallel 8 processes:  0.278218, 0.316637, 0.265009 = 0.286
parallel 7 processes: 0.267927, 0.245304, 0.272142 = 0.261
parallel 6 processes: 0.286661, 0.246757, 0.310080 = 0.281
parallel 5 processes: 0.286422, 0.261947, 0.242098 = 0.263
parallel 4 processes: 0.336291
parallel 3 processes:
parallel 1 processes:

when running parallel on 4 and 3 processes, the picture is assembled correctly, but the error occurs. 2 processes does not work at all

openMP 10 processes: 0.467,  0.474, 0.464 = 0.468
openMP 9 processes: 0.553, 0.423, 0.472 = 0.482
openMP 8 processes: 0.423760, 0.457275, 0.635804 = 0.505
openMP 7 processes: 0.380886, 0.347691, 0.381516 = 0.370
openMP 6 processes: 0.436595, 0.415818, 0.417 = 0.423
openMP 5 processes: 0.417710, 0.380704, 0.419535 = 0.405
openMP 4 processes: 0.312877,  0.323461, (picture is fully assembled here. why???) = 0.318
openMP 3 processes:
openMP 2 processes: 2 processes does not work at all
openMP 1 processes:

"""
