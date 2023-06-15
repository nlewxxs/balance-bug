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
RUN apt-get install git -y

ENV GOROOT="/home/go"
ENV GOPATH="/usr/src/"
ENV PATH="$GOPATH/bin:$GOROOT/bin:$PATH"
#RUN echo "export GOROOT=/usr/local/go" >> ~/.profile
#RUN echo "export PATH=$GOPATH/bin:$GOROOT/bin:$PATH" >> ~/.profile
# RUN echo "export GOROOT=/usr/local/go" >> /etc/bash.bashrc
# RUN echo "export PATH=$GOPATH/bin:$GOROOT/bin:$PATH" >> /etc/bash.bashrc

WORKDIR /home/go/src
COPY ./adder ./

WORKDIR /usr/src/app
COPY ./ .
#RUN go build -v -o /usr/local/bin/app ./...

ENTRYPOINT [ "/bin/bash" ]
#CMD ["app"]