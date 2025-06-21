import os
from pathlib import Path

cwd = Path.cwd()
parent = cwd.parent

# Set the directory and file extension to search for
directories = ['piscript\\builtin','piscript\\test','piscript']

extensions = [".c", ".h", ".pi"]

# Initialize a variable to store the total number of lines
total_lines = 0

# Get the previous count from the file
previous_count = 0
count_file_path = os.path.join(cwd, 'line_count.txt')

if os.path.exists(count_file_path):
    with open(count_file_path, 'r') as count_file:
        lines = count_file.read().strip().split('\n')
        if lines:
            previous_count = int(lines[-1])

for extension in extensions:
    for dir in directories:    
        directory = os.path.join(os.path.dirname(parent), dir)
        # Loop through all files in the directory
        for filename in os.listdir(directory):
            # Check if the file has the specified extension
            if filename.endswith(extension):
                # Open the file and count the number of lines
                with open(os.path.join(directory, filename), encoding='utf-8', errors='ignore') as f:
                    num_lines = sum(1 for line in f)
                    total_lines += num_lines

# Store the current count in the file
with open(count_file_path, 'a') as count_file:
    count_file.write(str(total_lines) + '\n')

# Calculate the difference from the previous count
difference = total_lines - previous_count

# Print the total number of lines and the difference
print(
    f"Total number of lines in {extensions} files in {directory}: {total_lines}")
print(f"Difference from previous run: {difference} lines")
