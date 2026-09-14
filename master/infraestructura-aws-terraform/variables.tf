# variables.tf

variable "region" {
  description = "Región de AWS donde desplegar los recursos"
  type        = string
  default     = "eu-north-1"
}

variable "allowed_ssh_cidr" {
  description = "CIDR permitido para acceso SSH"
  type        = string
  default     = "0.0.0.0/0"
}

variable "ami_amazon_linux" {
  description = "AMI de Amazon Linux 2023 en eu-north-1"
  type        = string
  default     = "ami-0b46816ffa1234887" # Actualiza con el resultado del comando
}

variable "ami_ubuntu" {
  description = "AMI de Ubuntu 22.04 LTS en eu-north-1"
  type        = string
  default     = "ami-0c33fcb753a7176f6" # Actualiza con el resultado del comando
}

variable "ami_redhat" {
  description = "AMI de Red Hat Enterprise Linux en eu-north-1"
  type        = string
  default     = "ami-08f4f3b20b13a157b" # Actualiza con el resultado del comando
}
