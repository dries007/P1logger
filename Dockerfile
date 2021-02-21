FROM python:3.9-alpine

LABEL org.opencontainers.image.source https://github.com/dries007/P1logger

RUN apk add --no-cache tzdata

WORKDIR /app

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY P1logger ./P1logger

CMD [ "python", "-m", "P1logger" ]
