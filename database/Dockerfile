<<<<<<< HEAD
FROM mysql


ENV MYSQL_DATABASE=mysql_dump \
    MYSQL_ROOT_PASSWORD=1234

ADD mysql_dump.sql /docker-entrypoint-initdb.d

EXPOSE 3306
=======
FROM mysql

ENV MYSQL_DB todo
ENV MYSQL_USER server
ENV MYSQL_ROOT_PASSWORD test_pwd

COPY mysql_dump.sql /docker-entrypoint-initdb.d/
>>>>>>> b49d1e4ef260c43a6960a1dda7a60156ca7f1dd8
