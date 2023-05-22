FROM mysql


ENV MYSQL_DATABASE=mysql_dump \
    MYSQL_ROOT_PASSWORD=1234

ADD mysql_dump.sql /docker-entrypoint-initdb.d

EXPOSE 3306
