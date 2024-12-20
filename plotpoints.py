import argparse
import matplotlib.pyplot as plt

# Function to read points from the file
def read_points_from_file(filename):
    points = []
    with open(filename, "r") as file:
        # Skip the first line (the number of points)
        file.readline()
        # Read the coordinates of each point
        for line in file:
            x, y = map(int, line.split())
            points.append((x, y))
    return points

# Function to plot points
def plot_points(points):
    # Unzip points into x and y coordinates
    x_coords, y_coords = zip(*points)
    
    # Create the plot
    plt.scatter(x_coords, y_coords, c='blue', marker='o')
    plt.title("Points Plot")
    plt.xlabel("X Coordinate")
    plt.ylabel("Y Coordinate")
    plt.grid(True)
    
    # Show the plot
    plt.show()

def main():
    # Argument parser setup
    parser = argparse.ArgumentParser(description="Plot points from a K-means test case file.")
    parser.add_argument("input_file", help="Path to the test case file containing the points.")
    
    args = parser.parse_args()

    # Read the points from the specified file
    points = read_points_from_file(args.input_file)

    # Plot the points
    plot_points(points)

if __name__ == "__main__":
    main()
