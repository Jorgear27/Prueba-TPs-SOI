# Paranoid Server

## Requirements

- PostgreSQL
- Conan 2.x
- CMake ≥ 3.10
- g++ ≥ 11

## Step 1: Prepare the Database

Desde Paranoid-Linux/server

```bash
sudo -u postgres psql -f init/init_postgres.sql
sudo -u postgres psql
CREATE USER server WITH PASSWORD 'server123';
\c paranoid_db
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO server;
\q
```

## Step 2: Compile the Project
Desde el directorio raiz

``` bash
    mkdir build
    conan install . --build=missing --settings=build_type=Debug
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=./build/Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
    make -j$(nproc)
    ./server/server
```
