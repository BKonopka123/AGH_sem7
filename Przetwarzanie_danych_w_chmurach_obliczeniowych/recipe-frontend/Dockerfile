# Użyj oficjalnego obrazu Nginx jako obrazu bazowego
FROM nginx:alpine

# Skopiuj pliki statyczne do katalogu Nginx
COPY . /usr/share/nginx/html

# Udostępnij port, na którym działa Nginx
EXPOSE 80

# Domyślne polecenie do uruchomienia Nginx
CMD ["nginx", "-g", "daemon off;"]