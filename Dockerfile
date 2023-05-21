FROM mysql

ENV MYSQL_DB todo
ENV MYSQL_USER server
ENV MYSQL_ROOT_PASSWORD test_pwd

COPY mysql_dump.sql /docker-entrypoint-initdb.d/