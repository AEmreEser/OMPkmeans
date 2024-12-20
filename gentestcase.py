import numpy as np
import matplotlib.pyplot as plt
import argparse

def generate_clusters(n_clusters, n_points, dist_type='gaussian', spread=1.0, cluster_distance=10.0, x_range=(-10, 10), y_range=(-10, 10), seed=None):
    """
    Generate test case data for k-means clustering with discrete clusters far apart.
    
    Parameters:
    - n_clusters: Number of clusters.
    - n_points: Total number of points to generate.
    - dist_type: Type of distribution ('gaussian', 'uniform', 'poisson').
    - spread: How spread out the points are within each cluster (applicable to 'gaussian' and 'uniform').
    - cluster_distance: Distance between cluster centers.
    - x_range: Range of x-values for points (min, max).
    - y_range: Range of y-values for points (min, max).
    - seed: Random seed for reproducibility.
    
    Returns:
    - points: Array of generated points.
    - labels: Array of labels indicating cluster assignment.
    """
    if seed is not None:
        np.random.seed(seed)
    
    points = []
    labels = []
    
    # Spread cluster centers across both axes
    cluster_centers = np.array([(i * cluster_distance, (i % 2) * cluster_distance) for i in range(n_clusters)])
    
    for i, center in enumerate(cluster_centers):
        if dist_type == 'gaussian':
            # Generate points from a Gaussian distribution around the cluster center
            cluster_points = np.random.normal(loc=center, scale=spread, size=(n_points // n_clusters, 2))
        elif dist_type == 'uniform':
            # Generate points from a uniform distribution within a box around the cluster center
            cluster_points = np.random.uniform(low=center-spread, high=center+spread, size=(n_points // n_clusters, 2))
        elif dist_type == 'poisson':
            # Generate points with Poisson distribution (for both x and y axis)
            cluster_points = np.random.poisson(lam=spread, size=(n_points // n_clusters, 2)) + center
        else:
            raise ValueError(f"Unsupported distribution type: {dist_type}")
        
        # Ensure points are within the specified x and y ranges and convert to integers
        cluster_points[:, 0] = np.clip(np.round(cluster_points[:, 0]), x_range[0], x_range[1]).astype(int)
        cluster_points[:, 1] = np.clip(np.round(cluster_points[:, 1]), y_range[0], y_range[1]).astype(int)
        
        # Append points and corresponding labels
        points.append(cluster_points)
        labels.append(np.full((n_points // n_clusters,), i))
    
    # Combine all points and labels
    points = np.vstack(points)
    labels = np.concatenate(labels)
    
    return points, labels

def save_points_to_file(points, file_name):
    """
    Save the generated points to a file, with the first line containing the number of points
    and each subsequent line containing the coordinates <x> <y>.
    
    Parameters:
    - points: Array of points (2D coordinates).
    - file_name: The name of the file to save the points.
    """
    with open(file_name, 'w') as f:
        # Write the number of points on the first line
        f.write(f"{len(points)}\n")
        # Write each point's coordinates in the format "<x> <y>"
        for point in points:
            f.write(f"{int(point[0])} {int(point[1])}\n")
    print(f"Points saved to {file_name}")

def plot_clusters(points, labels, title="Generated Clusters"):
    """
    Plot the generated clusters with labels.
    
    Parameters:
    - points: Array of points (2D coordinates).
    - labels: Array of cluster labels.
    """
    plt.figure(figsize=(8, 6))
    plt.scatter(points[:, 0], points[:, 1], c=labels, cmap='viridis', marker='o', s=30)
    plt.title(title)
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.colorbar(label='Cluster Label')
    plt.grid(True)
    plt.show()
def main():
    # Parse CLI arguments
    parser = argparse.ArgumentParser(description="Generate synthetic test data for k-means clustering.")
    
    parser.add_argument('--nclusters', type=int, required=True, help="Number of clusters.")
    parser.add_argument('--npoints', type=int, required=True, help="Total number of points.")
    parser.add_argument('--disttype', choices=['gaussian', 'uniform', 'poisson'], default='gaussian', 
                        help="Type of distribution for generating points ('gaussian', 'uniform', or 'poisson').")
    parser.add_argument('--spread', type=float, default=1.0, help="Spread of points within each cluster.")
    parser.add_argument('--clusterdistance', type=float, default=10.0, help="Distance between cluster centers.")
    parser.add_argument('--seed', type=int, help="Random seed for reproducibility.")
    parser.add_argument('--xrange', type=str, default='-10,10', help="Range for x-values (min,max). Example: '-5,5'")
    parser.add_argument('--yrange', type=str, default='-10,10', help="Range for y-values (min,max). Example: '-5,5'")
    parser.add_argument('--outputfile', type=str, required=True, help="Name of the file to save the generated points.")
    

    args = parser.parse_args()

    xrange = tuple(map(float, args.xrange.split(',')))
    yrange = tuple(map(float, args.yrange.split(',')))
    
    # Generate clusters based on arguments
    points, labels = generate_clusters(args.nclusters, args.npoints, args.disttype, 
                                       args.spread, args.clusterdistance, xrange, yrange, args.seed)

    save_points_to_file(points, args.outputfile)
    
    # Plot the generated clusters
    # plot_clusters(points, labels, title=f"Clusters with {args.disttype.capitalize()} Distribution")

if __name__ == '__main__':
    main()
