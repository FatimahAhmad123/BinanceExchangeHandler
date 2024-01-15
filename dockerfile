# Use the latest Ubuntu image as the base
FROM ubuntu:latest

# Update package list and install necessary dependencies
RUN apt-get update && \
	apt-get install -y cmake g++ make git libssl-dev libboost1.74-tools-dev && \
	rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /app

# Start the container in interactive mode
CMD ["/bin/bash"]
