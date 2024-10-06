FROM debian:12-slim

# アプリ実行時のユーザをrootではないuserを作る
RUN groupadd -g 1000 user && \
    useradd -r -u 1000 -g user user

# ryeのインストールに必要なツールのインストール
RUN apt-get update \
      && apt-get -y install curl \
      && apt clean

# ryeのインストールと、パスの設定
ARG RYE_VERSION=0.41.0
ENV RYE_HOME="/usr/local/share/rye"
ENV PATH="$RYE_HOME/shims:$PATH"

RUN curl -sSfL https://rye-up.com/get | \
    RYE_VERSION=${RYE_VERSION} \
    RYE_NO_AUTO_INSTALL=1 \
    RYE_INSTALL_OPTION="--yes" \
    bash

# アプリケーションディレクトリに移動
WORKDIR /app

# モジュールのインストール
COPY .python-version \
    requirements.lock \
    requirements-dev.lock \
    pyproject.toml \
    .

RUN rye sync --no-dev --no-lock

COPY src src

USER 1000

CMD ["rye", "run", "streamlit", "run", "src/app.py", "--server.address=0.0.0.0", "--server.port=5831"]