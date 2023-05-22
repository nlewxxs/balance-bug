FROM mysql


ENV MYSQL_DATABASE=testapp \
    MYSQL_ROOT_PASSWORD=helloworld

ADD mysql_dump.sql /docker-entrypoint-initdb.d

EXPOSE 3306
