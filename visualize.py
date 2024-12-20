import matplotlib.pyplot as plt
import sys

# Function to parse the cluster output from the text
def parse_clusters(output):
    clusters = []
    lines = output.splitlines()
    
    i = 0
    while i < len(lines):
        # Extract cluster center information (7th line and following)
        if lines[i].startswith("**Cluster"):
            # Extract center coordinates from the cluster info line
            cluster_info = lines[i].strip().split("Center :")[1].strip()[1:-1]
            center_x, center_y = map(float, cluster_info.split(","))
            
            # Move to the next line, which will contain the points for this cluster
            points = []
            i += 1
            # Collect points until we reach the next cluster header or the end
            while i < len(lines) and lines[i].startswith("("):
                # Clean and split each point
                points_line = lines[i].strip()
                points_str = points_line.split(") (")  # Split points separated by a space
                # Parse each point
                for point in points_str:
                    point_info = point.strip('()')  # Remove parentheses
                    point_x, point_y = map(int, point_info.split(","))
                    points.append((point_x, point_y))
                i += 1
            
            # Append the cluster center and its points to the clusters list
            clusters.append((center_x, center_y, points))
        else:
            i += 1  # Skip any lines that do not contain cluster info
    
    return clusters

# Function to plot the points and clusters
def plot_clusters(clusters):
    # Create a plot
    plt.figure(figsize=(10, 6))
    
    # Different colors for different clusters
    colors = ['r', 'g', 'b', 'c', 'm', 'y', 'k']
    
    # Iterate through clusters and plot
    for idx, (center_x, center_y, points) in enumerate(clusters):
        # Plot the cluster center
        plt.scatter(center_x, center_y, color=colors[idx % len(colors)], marker='X', s=100, label=f'Cluster {idx} Center')
        
        # Plot the points in the cluster
        if points:  # Ensure there are points in the cluster before plotting
            points_x = [p[0] for p in points]
            points_y = [p[1] for p in points]
            plt.scatter(points_x, points_y, color=colors[idx % len(colors)], marker='o', label=f'Cluster {idx} Points')
    
    # Adjust the axis to fit the data
    all_x = [point[0] for cluster in clusters for point in cluster[2]]
    all_y = [point[1] for cluster in clusters for point in cluster[2]]
    
    if all_x and all_y:
        padding = 50  # add some padding to the plot boundaries
        min_x, max_x = min(all_x), max(all_x)
        min_y, max_y = min(all_y), max(all_y)
        
        plt.xlim(min_x - padding, max_x + padding)
        plt.ylim(min_y - padding, max_y + padding)
    
    plt.xlabel('X Coordinate')
    plt.ylabel('Y Coordinate')
    plt.title('K-Means Clusters and Points')
    plt.grid(True)

# Main code to run
def main():
    # Check if a file name was provided as a command-line argument
    if len(sys.argv) != 2:
        print("Usage: python script.py <cluster_output_file>")
        sys.exit(1)
    
    # Get the file name from the command-line argument
    file_name = sys.argv[1]
    
    try:
        # Open and read the file
        with open(file_name, 'r') as file:
            output = file.read()
        
        # Parse the output from the file
        clusters = parse_clusters(output)

        # Plot the clusters and points
        plot_clusters(clusters)

        # Save the plot as a file named by the number of clusters
        num_clusters = len(clusters)
        plot_filename = f'kmeans_clusters_{num_clusters}_clusters.png'
        plt.savefig(plot_filename)
        print(f"Plot saved as {plot_filename}")
        plt.close()

    except FileNotFoundError:
        print(f"Error: The file '{file_name}' was not found.")
        sys.exit(1)

# Run the main function
if __name__ == "__main__":
    main()
