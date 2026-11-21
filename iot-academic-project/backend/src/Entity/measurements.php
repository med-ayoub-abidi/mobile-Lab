<?php

namespace App\Entity;

use ApiPlatform\Metadata\ApiResource;
use App\Repository\MeasurementRepository;
use Doctrine\DBAL\Types\Types;
use Doctrine\ORM\Mapping as ORM;

#[ORM\Entity(repositoryClass: MeasurementRepository::class)]
#[ORM\Table(name: 'measurements')]
#[ApiResource]
class measurements
{
    #[ORM\Id]
    #[ORM\GeneratedValue]
    #[ORM\Column]
    private ?int $id = null;

    #[ORM\Column(nullable: true)]
    private ?int $sensor_id = null;

    #[ORM\Column(type: Types::FLOAT, nullable: true)]
    private ?float $valve = null;

    #[ORM\Column(type: Types::DATETIME_IMMUTABLE, nullable: true)]
    private ?\DateTimeImmutable $measured_at = null;

    #[ORM\Column(type: Types::DATETIMETZ_IMMUTABLE, nullable: true)]
    private ?\DateTimeImmutable $created_at = null;

    public function getId(): ?int
    {
        return $this->id;
    }

    public function getSensorId(): ?int
    {
        return $this->sensor_id;
    }

    public function setSensorId(?int $sensorId): static
    {
        $this->sensor_id = $sensorId;
        return $this;
    }

    public function getValve(): ?float
    {
        return $this->valve;
    }

    public function setValve(?float $valve): static
    {
        $this->valve = $valve;
        return $this;
    }

    public function getMeasuredAt(): ?\DateTimeImmutable
    {
        return $this->measured_at;
    }

    public function setMeasuredAt(?\DateTimeImmutable $measuredAt): static
    {
        $this->measured_at = $measuredAt;
        return $this;
    }

    public function getCreatedAt(): ?\DateTimeImmutable
    {
        return $this->created_at;
    }

    public function setCreatedAt(?\DateTimeImmutable $createdAt): static
    {
        $this->created_at = $createdAt;
        return $this;
    }
}
