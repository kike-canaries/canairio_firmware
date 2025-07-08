FROM python:3.9.0-slim

ENV APP_VERSION="6.1.18" \
    APP="platformio-core"

LABEL app.name="${APP}" \
      app.version="${APP_VERSION}" \
      maintainer="Hpsaturn <@hpsaturn>"

WORKDIR /workspace

RUN pip install -U platformio==${APP_VERSION} && \
    mkdir -p /workspace && \
    mkdir -p /.platformio && \
    chmod a+rwx /.platformio && \
    apt-get update && apt-get install git -y && \
    apt-get clean && rm -rf /var/tmp/*

# user config:
ARG DOCKER_USER=default_user
ARG DOCKER_USERID=default_userid
# Cannot run as root, we'll just do everything else as a user
# The dialup group maybe doesn't work in Docker. Please help. Issue #10 
RUN chmod a+rwx /workspace && \ 
    useradd -d /workspace -u $DOCKER_USERID $DOCKER_USER && \
    chown $DOCKER_USER:$DOCKER_USER /workspace && \
    usermod -a -G dialout $DOCKER_USER

USER $DOCKER_USER

ENTRYPOINT ["platformio"]
