<?php

declare(strict_types=1);

namespace DoctrineMigrations;

use Doctrine\DBAL\Schema\Schema;
use Doctrine\Migrations\AbstractMigration;

/**
 * Auto-generated Migration: Please modify to your needs!
 */
final class Version20261125184918 extends AbstractMigration
{
    public function getDescription(): string
    {
        return '';
    }

    public function up(Schema $schema): void
    {
        // this up() migration is auto-generated, please modify it to your needs
        $this->addSql('ALTER TABLE measurements ADD value DOUBLE PRECISION NOT NULL');
        $this->addSql('ALTER TABLE measurements DROP valve');
        $this->addSql('ALTER TABLE measurements ALTER sensor_id SET NOT NULL');
        $this->addSql('ALTER TABLE measurements ALTER measured_at TYPE TIMESTAMP(0) WITH TIME ZONE');
        $this->addSql('ALTER TABLE measurements ALTER measured_at SET NOT NULL');
        $this->addSql('ALTER TABLE measurements ALTER created_at SET DEFAULT CURRENT_TIMESTAMP');
        $this->addSql('ALTER TABLE measurements ALTER created_at SET NOT NULL');
        $this->addSql('CREATE UNIQUE INDEX UNIQ_MEASUREMENT_SENSOR_TIME ON measurements (sensor_id, measured_at)');
    }

    public function down(Schema $schema): void
    {
        // this down() migration is auto-generated, please modify it to your needs
        $this->addSql('DROP INDEX UNIQ_MEASUREMENT_SENSOR_TIME');
        $this->addSql('ALTER TABLE measurements ADD valve DOUBLE PRECISION DEFAULT NULL');
        $this->addSql('ALTER TABLE measurements DROP value');
        $this->addSql('ALTER TABLE measurements ALTER sensor_id DROP NOT NULL');
        $this->addSql('ALTER TABLE measurements ALTER measured_at TYPE TIMESTAMP(0) WITHOUT TIME ZONE');
        $this->addSql('ALTER TABLE measurements ALTER measured_at DROP NOT NULL');
        $this->addSql('ALTER TABLE measurements ALTER created_at DROP DEFAULT');
        $this->addSql('ALTER TABLE measurements ALTER created_at DROP NOT NULL');
    }
}
