# FROM golang:alpine AS builder

# RUN apk update && apk add --no-cache git 

# RUN mkdir /build

# WORKDIR /build

# COPY . .

# RUN go mod download

# RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -v -a -installsuffix cgo -o /go/bin/build


# FROM scratch

# COPY --from=builder /go/bin/build /go/bin/build
# COPY --from=builder /build/wait-for-it.sh ./wait-for-it.sh

# CMD ["chmod", "+x", "wait-for-it.sh"]
# CMD ["./wait-for-it.sh", "db:5432", "--"]

# ENTRYPOINT ["/go/bin/build"]

# EXPOSE 8081
FROM ubuntu:22.04

WORKDIR /tmp
RUN apt-get update

RUN apt-get install wget -y
RUN apt-get install curl -y
RUN wget  https://go.dev/dl/go1.20.2.linux-amd64.tar.gz 
RUN tar -xvf go1.20.2.linux-amd64.tar.gz   


RUN mv go /home
# RUN export GOROOT=/usr/local/go
# RUN export PATH=$GOPATH/bin:$GOROOT/bin:$PATH 
WORKDIR /home
RUN apt-get install gnupg2 -y
RUN curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
RUN echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list
RUN curl -sL https://deb.nodesource.com/setup_14.x | bash -
RUN apt-get update

RUN apt-get install yarn -y

RUN apt-get install nodejs -y
RUN apt-get install build-essential -y
RUN apt-get install git

ENV GOROOT="/home/go"
ENV GOPATH="/usr/src/"
ENV PATH="$GOPATH/bin:$GOROOT/bin:$PATH"
#RUN echo "export GOROOT=/usr/local/go" >> ~/.profile
#RUN echo "export PATH=$GOPATH/bin:$GOROOT/bin:$PATH" >> ~/.profile
# RUN echo "export GOROOT=/usr/local/go" >> /etc/bash.bashrc
# RUN echo "export PATH=$GOPATH/bin:$GOROOT/bin:$PATH" >> /etc/bash.bashrc

WORKDIR /usr/src/app
COPY . .
#RUN go build -v -o /usr/local/bin/app ./...

ENTRYPOINT [ "/bin/bash" ]
#CMD ["app"]
