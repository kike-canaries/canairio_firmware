FROM python:3.9.0-slim

ENV APP_VERSION="6.0.2" \
    APP="platformio-core"

LABEL app.name="${APP}" \
      app.version="${APP_VERSION}" \
      maintainer="Hpsaturn <@hpsaturn>"

RUN pip install -U platformio==${APP_VERSION} && \
    mkdir -p /workspace && \
    mkdir -p /.platformio && \
    chmod a+rwx /.platformio && \
    apt-get update && apt-get install git -y && \
    apt-get clean && rm -rf /var/tmp/*


USER 1001

WORKDIR /workspace

ENTRYPOINT ["platformio"]
