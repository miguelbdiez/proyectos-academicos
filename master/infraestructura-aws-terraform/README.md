# Infraestructura AWS con Terraform

Despliegue reproducible de una pequeña infraestructura en AWS descrita íntegramente como
código: tres instancias EC2 con **tres distribuciones Linux distintas**, cada una con su par
de claves SSH y su grupo de seguridad.

Asignatura: *DAIITI* — Máster Universitario en Ingeniería Informática (USAL).

---

## Qué despliega

| Recurso | Cantidad | Detalle |
|---|---|---|
| `aws_instance` | 3 | Amazon Linux 2023, Ubuntu 22.04 LTS y Red Hat Enterprise Linux |
| `aws_key_pair` | 3 | Una clave SSH independiente por instancia |
| `aws_security_group` | 3 | Reglas de entrada distintas según el rol de cada máquina |

Región por defecto: `eu-north-1` (Estocolmo).

Los `outputs` exponen la IP pública de cada instancia, su ficha completa de detalles y un
resumen agregado de las tres, de modo que tras el `apply` se dispone de todo lo necesario
para conectarse sin consultar la consola de AWS.

---

## Uso

### Requisitos

- Terraform 1.5 o superior
- AWS CLI configurado (`aws configure`); el proveedor toma las credenciales de ahí, no del código

### Claves SSH

El código espera `key1.pub`, `key2.pub` y `key3.pub` en el directorio del módulo. Genéralas:

```bash
for i in 1 2 3; do ssh-keygen -t ed25519 -f key$i -N "" -C "key$i"; mv key$i.pub .; done
```

> Las claves originales de la entrega no se incluyen en el repositorio.

### AMI

Los identificadores de AMI en [`variables.tf`](variables.tf) son de `eu-north-1` y caducan.
Actualízalos antes de desplegar:

```bash
aws ec2 describe-images --owners amazon \
  --filters "Name=name,Values=al2023-ami-2023.*-x86_64" \
  --query 'sort_by(Images,&CreationDate)[-1].ImageId' --output text
```

### Despliegue

```bash
terraform init
terraform plan
terraform apply
terraform destroy    # al terminar, para no incurrir en coste
```

`allowed_ssh_cidr` viene con `0.0.0.0/0` por comodidad en el aula. **Restríngelo a tu IP**
antes de usarlo en cualquier entorno real.

---

Memoria de la práctica en [`docs/terraform-DAIITI.pdf`](docs/terraform-DAIITI.pdf).
