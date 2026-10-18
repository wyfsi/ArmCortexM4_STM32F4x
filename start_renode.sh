#!/bin/bash

# Target and kill only external processes listening on ports 1234 or 3333
PORT_PIDS=$(lsof -t -i:1234 -i:3333 2>/dev/null)
if [ -n "$PORT_PIDS" ]; then
    kill -9 $PORT_PIDS 2>/dev/null || true
fi

# Kill any lingering dotnet/Renode instances safely
pkill -9 -f "Renode.dll" 2>/dev/null || true

sleep 0.5

# Launch Renode in background
nohup renode --disable-xwt stm32f4.resc > renode.log 2>&1 &

echo "Waiting for Renode GDB stub on localhost:3333..."

# Wait until TCP port 3333 is open
while ! nc -z 127.0.0.1 3333 2>/dev/null; do
  sleep 0.1
done

echo "Renode GDB server is live on localhost:3333!"