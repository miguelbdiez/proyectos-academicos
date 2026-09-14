# outputs.tf

# ============================================
# PARTE 5: Outputs - IPs Públicas
# ============================================

output "vm1_public_ip" {
  description = "IP pública de la instancia EC2-1"
  value       = aws_instance.vm1.public_ip
}

output "vm2_public_ip" {
  description = "IP pública de la instancia EC2-2"
  value       = aws_instance.vm2.public_ip
}

output "vm3_public_ip" {
  description = "IP pública de la instancia EC2-3"
  value       = aws_instance.vm3.public_ip
}

# ============================================
# Outputs extendidos - Información adicional
# ============================================

# Información completa de VM1
output "vm1_details" {
  description = "Detalles completos de EC2-1"
  value = {
    instance_name  = "EC2-1"
    public_ip      = aws_instance.vm1.public_ip
    instance_type  = aws_instance.vm1.instance_type
    key_name       = aws_instance.vm1.key_name
    security_group = aws_security_group.securitygroup1.name
  }
}

# Información completa de VM2
output "vm2_details" {
  description = "Detalles completos de EC2-2"
  value = {
    instance_name  = "EC2-2"
    public_ip      = aws_instance.vm2.public_ip
    instance_type  = aws_instance.vm2.instance_type
    key_name       = aws_instance.vm2.key_name
    security_group = aws_security_group.securitygroup2.name
  }
}

# Información completa de VM3
output "vm3_details" {
  description = "Detalles completos de EC2-3"
  value = {
    instance_name  = "EC2-3"
    public_ip      = aws_instance.vm3.public_ip
    instance_type  = aws_instance.vm3.instance_type
    key_name       = aws_instance.vm3.key_name
    security_group = aws_security_group.securitygroup3.name
  }
}

# Resumen de todas las instancias
output "all_instances_summary" {
  description = "Resumen de todas las instancias creadas"
  value = {
    vm1 = {
      name           = "EC2-1"
      ip             = aws_instance.vm1.public_ip
      key            = aws_key_pair.key1.key_name
      security_group = aws_security_group.securitygroup1.name
    }
    vm2 = {
      name           = "EC2-2"
      ip             = aws_instance.vm2.public_ip
      key            = aws_key_pair.key2.key_name
      security_group = aws_security_group.securitygroup2.name
    }
    vm3 = {
      name           = "EC2-3"
      ip             = aws_instance.vm3.public_ip
      key            = aws_key_pair.key3.key_name
      security_group = aws_security_group.securitygroup3.name
    }
  }
}