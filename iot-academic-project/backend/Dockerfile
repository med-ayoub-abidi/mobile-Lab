# Production image for the Symfony API. Local development uses the same file
# through docker-compose.yml (which mounts the source over /var/www/html).
FROM php:8.4-apache

# System dependencies and PHP extensions for Symfony/PostgreSQL.
RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    unzip \
    libpq-dev \
    libicu-dev \
    && docker-php-ext-install intl opcache pdo pdo_pgsql \
    && rm -rf /var/lib/apt/lists/*

# Apache: serve Symfony's public/ directory, allow .htaccess routing, and
# preserve the Authorization header so JWT bearer tokens reach Symfony.
ENV APACHE_DOCUMENT_ROOT=/var/www/html/public
RUN a2enmod rewrite setenvif \
    && echo 'SetEnvIf Authorization "(.*)" HTTP_AUTHORIZATION=$1' \
        > /etc/apache2/conf-available/authorization.conf \
    && a2enconf authorization \
    && sed -ri -e "s!/var/www/html!${APACHE_DOCUMENT_ROOT}!g" /etc/apache2/sites-available/*.conf \
    && sed -ri -e "s!/var/www/!${APACHE_DOCUMENT_ROOT}!g" /etc/apache2/apache2.conf /etc/apache2/conf-available/*.conf \
    && sed -ri "s/AllowOverride None/AllowOverride All/g" /etc/apache2/apache2.conf

WORKDIR /var/www/html

COPY --from=composer:2.8 /usr/bin/composer /usr/bin/composer

# Production defaults. Render overrides secrets and DATABASE_URL at runtime.
ENV APP_ENV=prod \
    APP_DEBUG=0 \
    COMPOSER_ALLOW_SUPERUSER=1

COPY . .

# Install dependencies at build time. Scripts are skipped on purpose: the
# runtime entrypoint warms the cache after JWT keys and the database exist.
RUN composer install \
        --no-dev \
        --no-interaction \
        --prefer-dist \
        --optimize-autoloader \
        --no-scripts

COPY docker-entrypoint.sh /usr/local/bin/render-entrypoint.sh
RUN chmod +x /usr/local/bin/render-entrypoint.sh

EXPOSE 80

CMD ["/usr/local/bin/render-entrypoint.sh"]
