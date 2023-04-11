import random

min_weight = 20
max_weight = 120
infinity_magic_value = 2147483647
num_edges = 4
n = 5  # amount of graph nodes

# Initialize an n x n matrix with 1000
adj_matrix = [[infinity_magic_value for j in range(n)] for i in range(n)]

# Generate a random connected graph with the specified number of edges
connected = False
while not connected:
    # Generate random values for the upper triangle
    edges = []
    for i in range(n):
        for j in range(i + 1, n):
            edges.append((i, j))

    random.shuffle(edges)

    for i in range(num_edges):
        edge = edges[i]
        adj_matrix[edge[0]][edge[1]] = random.randint(min_weight, max_weight)

    # Check if the graph is connected
    visited = [False] * n
    stack = [0]
    visited[0] = True
    while stack:
        node = stack.pop()
        for neighbor in range(n):
            if adj_matrix[node][neighbor] < infinity_magic_value and not visited[neighbor]:
                visited[neighbor] = True
                stack.append(neighbor)

    if all(visited):
        connected = True

# Print the matrix
for row in adj_matrix:
    print(row)


with open("Matrix.txt", "w") as matrix_file:
    matrix_file.write(f"{n}\n")
    for i, row in enumerate(adj_matrix):
        for j, value in enumerate(row):
            if j > i:
                matrix_file.write(f"{value}\n")
print("end")
