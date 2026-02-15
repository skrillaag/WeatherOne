# Weather App

A full-stack weather application consisting of:

    - A C++ backend (CLI + HTTP server) built with CMake and vcpkg
    - A React frontend built with Vite
    - Real authentication, sessions, persistence, and live weather data

This project is intended as a real-world systems demo, not a toy or coursework exercise.

## Features

### Backend (C++)
- HTTP server using Boost.Beast / Boost.Asio
- User registration and login with hashed passwords
- Session-based authentication (Bearer tokens)
- SQLite persistence for users and query history
- Live weather data fetched over HTTPS
- CLI mode for direct terminal usage
- Proper CORS handling for browser clients

### Frontend (React)
- Login / registration UI
- Authenticated weather lookup
- Query history view
- Real backend integration
- Minimal styling

## Tech Stack
### Backend
- C++17
- CMake
- vcpkg (manifest mode)
- Boost (Beast, Asio)
- OpenSSL
- SQLite3
- nlohmann/json
### Frontend
- React
- Vite
- Fetch API

## Prerequisites
- Windows
- CMake ≥ 3.20
- vcpkg installed and integrated
- Node.js ≥ 18
- A free API key from https://www.weatherapi.com

## Setup

1. Clone repository
```powershell
cd WeatherAppCLI
```

2. Backend Build
```powershell
cmake -S . -B build
cmake --build build
```

3. Set the Weather API key
```powershell
$env:WEATHERAPI_KEY="your_weatherapi_key_here"
```

4. Run the backend server
```powershell
cd build\Debug
.\WeatherApp.exe --server 127.0.0.1 8080
```

You should see:
```powershell
Server running at http://127.0.0.1:8080
```

You can verify with:
http://127.0.0.1:8080/health

5. Frontend setup
Open a second terminal:
```
cd weather-react
npm install
npm run dev
```

Vite will output a local URL:
```
http://localhost:5173
```
Open it.

## Usage
1. Register a new user
2. Log in
3. Enter a city name to fetch
4. View query history


## CLI Mode
The backend can also be run as terminal application:
```powershell
.\WeatherApp.exe --cli
```

## Project Structure
```
WeatherAppCLI/
├─ src/ # C++ backend source
├─ weather-react/ # React frontend
├─ CMakeLists.txt
├─ vcpkg.json
├─ .gitignore
└─ README.md
```
