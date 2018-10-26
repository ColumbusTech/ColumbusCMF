#script to get number of C/C++ and Python code lines in project

import os

num_cpp_lines = int(0)
num_cpp_header_lines = int(0)
num_python_lines = int(0)

for root, dirs, files in os.walk("."):
	for filename in files:
		if filename.lower().endswith(('.c', '.cpp', 'cxx', 'c++', 'cc')):
			num_cpp_lines += sum(1 for line in open(os.path.join(root, filename)))
		if filename.lower().endswith(('.h', '.hpp', '.hxx', '.h++', '.hh')):
			num_cpp_header_lines += sum(1 for line in open(os.path.join(root, filename)))
		if filename.lower().endswith(('.py', '.pyw', '.pyc', '.pyo', '.pyd')):
			num_python_lines += sum(1 for line in open(os.path.join(root, filename)))


print('Number of C/C++ code lines: ', num_cpp_lines)
print('Number of C/C++ header code lines: ', num_cpp_header_lines)
print('Number of Python lines: ', num_python_lines)










