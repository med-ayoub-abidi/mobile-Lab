#!/bin/sh
# Runtime setup for both Render and local Docker Compose.
set -eu

cd /var/www/html

# Render injects PORT (default 10000). Make Apache listen on that port.
APACHE_PORT="${PORT:-80}"
sed -ri "s/^Listen .*/Listen ${APACHE_PORT}/" /etc/apache2/ports.conf
sed -ri "s/<VirtualHost \*:.*>/<VirtualHost *:${APACHE_PORT}>/" /etc/apache2/sites-available/000-default.conf

if [ -f vendor/autoload_runtime.php ]; then
    # JWT keys are git-ignored. Generate them on first boot unless Secret Files
    # are mounted and JWT_SECRET_KEY / JWT_PUBLIC_KEY point somewhere else.
    if [ -z "${JWT_SECRET_KEY:-}" ] || [ "${JWT_SECRET_KEY}" = '%kernel.project_dir%/config/jwt/private.pem' ]; then
        mkdir -p config/jwt
        if [ ! -f config/jwt/private.pem ] || [ ! -f config/jwt/public.pem ]; then
            echo "Generating JWT keypair..."
            php bin/console lexik:jwt:generate-keypair --skip-if-exists --no-interaction
        fi
    fi

    # Wait for the managed database, then apply migrations from the image.
    attempt=0
    until php bin/console doctrine:query:sql 'SELECT 1' --no-interaction >/dev/null 2>&1; do
        attempt=$((attempt + 1))
        if [ "$attempt" -ge 30 ]; then
            echo "Database is not reachable after 30 attempts." >&2
            exit 1
        fi
        echo "Waiting for database... ($attempt/30)"
        sleep 2
    done

    php bin/console doctrine:migrations:migrate --no-interaction --allow-no-migration
    php bin/console cache:clear --env=prod --no-interaction
else
    # Local docker-compose mounts ./backend over /var/www/html, which hides
    # the vendor/ directory baked into the image. Keep Apache up so that
    # `docker compose exec backend composer install` can be run afterwards.
    echo "vendor/ is missing from the mounted source; run: docker compose exec backend composer install"
fi

# Replace the shell so Apache receives SIGTERM from Render.
exec apache2-foreground
