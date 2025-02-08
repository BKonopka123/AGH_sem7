# Użyj oficjalnego obrazu Node.js jako obrazu bazowego
FROM node:14

# Ustaw katalog roboczy w kontenerze
WORKDIR /usr/src/app

# Skopiuj package.json i package-lock.json
COPY package*.json ./

# Zainstaluj zależności
RUN npm install

# Skopiuj resztę kodu aplikacji
COPY . .

# Udostępnij port, na którym działa aplikacja
EXPOSE 5000

# Polecenie do uruchomienia aplikacji
CMD ["node", "index.js"]