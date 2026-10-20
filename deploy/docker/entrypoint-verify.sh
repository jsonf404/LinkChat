#!/bin/sh
set -eu

node -e 'const fs = require("fs"); const config = {
  email: { user: process.env.SMTP_USER, pass: process.env.SMTP_PASSWORD, smtp_host: process.env.SMTP_HOST, smtp_port: Number(process.env.SMTP_PORT) },
  mysql: { host: process.env.MYSQL_HOST, port: Number(process.env.MYSQL_PORT) },
  redis: { host: process.env.REDIS_HOST, port: Number(process.env.REDIS_PORT), passwd: process.env.REDIS_PASSWORD }
};
for (const [key, value] of Object.entries({SMTP_USER: config.email.user, SMTP_PASSWORD: config.email.pass, MYSQL_HOST: config.mysql.host, REDIS_HOST: config.redis.host, REDIS_PASSWORD: config.redis.passwd})) {
  if (!value) { console.error(`Missing required environment variable: ${key}`); process.exit(1); }
}
fs.writeFileSync("/app/config.json", JSON.stringify(config));'

exec node /app/server.js
