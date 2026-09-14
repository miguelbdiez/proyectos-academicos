# main.tf


# ============================================
# PARTE 1: Proveedor AWS
# ============================================
terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region = var.region
  # Se usa AWS CLI configurado
}

# ============================================
# PARTE 2: Definición de claves SSH
# ============================================

resource "aws_key_pair" "key1" {
  key_name   = "key1"
  public_key = file("${path.module}/key1.pub")

  tags = {
    Name = "key1"
  }
}

resource "aws_key_pair" "key2" {
  key_name   = "key2"
  public_key = file("${path.module}/key2.pub")

  tags = {
    Name = "key2"
  }
}

resource "aws_key_pair" "key3" {
  key_name   = "key3"
  public_key = file("${path.module}/key3.pub")

  tags = {
    Name = "key3"
  }
}

# ============================================
# PARTE 3: Security Groups
# ============================================
# Hay 3 Security Groups: securitygroup1, securitygroup2, securitygroup3

# Security Group 1: SSH + HTTP (para EC2-1)
resource "aws_security_group" "securitygroup1" {
  name        = "securitygroup1"
  description = "Permite SSH y HTTP"

  # Regla de entrada: SSH
  ingress {
    description = "SSH from anywhere"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = [var.allowed_ssh_cidr]
  }

  # Regla de entrada: HTTP
  ingress {
    description = "HTTP from anywhere"
    from_port   = 80
    to_port     = 80
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # Regla de salida: Todo el tráfico
  egress {
    description = "All outbound traffic"
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = {
    Name = "securitygroup1"
  }
}

# Security Group 2: SSH + HTTPS
resource "aws_security_group" "securitygroup2" {
  name        = "securitygroup2"
  description = "Permite SSH y HTTPS"

  # Regla de entrada: SSH
  ingress {
    description = "SSH from anywhere"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = [var.allowed_ssh_cidr]
  }

  # Regla de entrada: HTTPS
  ingress {
    description = "HTTPS from anywhere"
    from_port   = 443
    to_port     = 443
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # Regla de salida: Todo el tráfico
  egress {
    description = "All outbound traffic"
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = {
    Name = "securitygroup2"
  }
}

# Security Group 3: SSH + MySQL
resource "aws_security_group" "securitygroup3" {
  name        = "securitygroup3"
  description = "Permite SSH y MySQL"

  # Regla de entrada: SSH
  ingress {
    description = "SSH from anywhere"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = [var.allowed_ssh_cidr]
  }

  # Regla de entrada: MySQL
  ingress {
    description = "MySQL from anywhere"
    from_port   = 3306
    to_port     = 3306
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # Regla de salida: Todo el tráfico
  egress {
    description = "All outbound traffic"
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = {
    Name = "securitygroup3"
  }
}

# ============================================
# PARTE 4: Instancias EC2
# ============================================

# Instancia 1: Amazon Linux - t3.small
resource "aws_instance" "vm1" {
  ami                    = var.ami_amazon_linux
  instance_type          = "t3.small"
  key_name               = aws_key_pair.key1.key_name
  vpc_security_group_ids = [aws_security_group.securitygroup1.id]

  tags = {
    Name = "EC2-1"
  }
}

# Instancia 2: Ubuntu - c7i-flex.large
resource "aws_instance" "vm2" {
  ami                    = var.ami_ubuntu
  instance_type          = "c7i-flex.large"
  key_name               = aws_key_pair.key2.key_name
  vpc_security_group_ids = [aws_security_group.securitygroup2.id]

  tags = {
    Name = "EC2-2"
  }
}

# Instancia 3: Red Hat - m7i-flex.large
resource "aws_instance" "vm3" {
  ami                    = var.ami_redhat
  instance_type          = "m7i-flex.large"
  key_name               = aws_key_pair.key3.key_name
  vpc_security_group_ids = [aws_security_group.securitygroup3.id]

  tags = {
    Name = "EC2-3"
  }
}