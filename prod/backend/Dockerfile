FROM ubuntu:22.04

WORKDIR /tmp
RUN apt-get update

WORKDIR /usr/src/app
COPY . .

EXPOSE 8081
RUN ["chmod", "+x", "./backend"]
ENTRYPOINT ["./backend"]

