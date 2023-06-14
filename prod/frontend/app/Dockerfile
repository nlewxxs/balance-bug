# Use an official Node.js runtime as the base image
FROM node:20-alpine

# Set the working directory inside the container
WORKDIR /app

# Copy the package.json and package-lock.json to the working directory
COPY package*.json .

# Install project dependencies
RUN npm install

# Copy the entire project directory to the working directory
COPY . .

# Install react-router-dom dependency
RUN npm install react-router-dom

# Install react-select dependency
RUN npm install react-select

# Expose port 3000 for the React development server
EXPOSE 3000

# Set the entrypoint to start the React development server
CMD ["npm", "start"]
