# Guide de Gestion de la Sécurité - Impulse Cover

## 🚨 Comprendre le Mode Sécurité

Le mode sécurité se déclenche automatiquement pour protéger votre installation dans ces situations :
- **Timeout dépassé** : Plus de 60 secondes (par défaut) d'activité continue
- **Cycles excessifs** : Plus de 5 cycles de direction (par défaut) en peu de temps
- **Protection matérielle** : Évite la surchauffe et l'usure prématurée

## ⚠️ Message d'Avertissement

Si vous voyez ce message dans vos logs :
```
[W][impulse_cover:137]: Cover is in safety mode, ignoring command
```

Cela signifie que le système de sécurité est actif et bloque temporairement les commandes.

## 🔧 Solutions pour Réinitialiser

### 1. Reset Manuel (Recommandé)

Ajoutez ce bouton physique dans votre configuration ESPHome :

```yaml
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO0  # Adaptez selon votre GPIO
      inverted: true
      mode:
        input: true
        pullup: true
    name: "Bouton Reset Sécurité"
    on_press:
      - impulse_cover.reset_safety:
          id: your_cover_id
      - logger.log: "🔧 Mode sécurité réinitialisé"
```

### 2. Service Home Assistant

Ajoutez ce service pour contrôler depuis Home Assistant :

```yaml
api:
  services:
    - service: reset_gate_safety
      then:
        - impulse_cover.reset_safety:
            id: your_cover_id
        - logger.log: "🔧 Sécurité réinitialisée via API"
```

Puis utilisez ce service dans Home Assistant :
```yaml
# Dans une automation Home Assistant
service: esphome.reset_gate_safety
target:
  entity_id: cover.your_cover
```

### 3. Reset Automatique

Pour un reset automatique pendant les heures creuses :

```yaml
time:
  - platform: homeassistant
    id: homeassistant_time

automation:
  - trigger:
      - platform: time
        at: "03:00:00"  # Chaque nuit à 3h
    action:
      - impulse_cover.reset_safety:
          id: your_cover_id
      - logger.log: "🌙 Reset automatique nocturne"
```

## ⚙️ Ajuster les Paramètres de Sécurité

Modifiez ces valeurs selon vos besoins :

```yaml
cover:
  - platform: impulse_cover
    name: "Mon Portail"
    id: my_gate
    # ... autres paramètres ...
    
    # Paramètres de sécurité personnalisés
    safety_timeout: 90s        # Augmenter si besoin de plus de temps
    safety_max_cycles: 3       # Réduire pour plus de sécurité
```

**Recommandations :**
- **safety_timeout** : 60-120s selon la taille de votre portail
- **safety_max_cycles** : 3-5 selon la fréquence d'utilisation

## 🔍 Diagnostic et Surveillance

### Logs à Surveiller

```bash
# Activation du mode sécurité
[W][impulse_cover]: Cover is in safety mode, ignoring command

# Reset réussi
[I][impulse_cover]: Safety mode reset successfully

# Cycle de sécurité incrémenté
[D][impulse_cover]: Safety cycle count: X/5
```

### Événements Home Assistant

Surveillez ces événements dans Home Assistant :

```yaml
automation:
  - alias: "Alerte Mode Sécurité"
    trigger:
      - platform: event
        event_type: esphome.safety_triggered
    action:
      - service: notify.mobile_app
        data:
          message: "🚨 Portail en mode sécurité"
```

## 🛠️ Dépannage Avancé

### Reset depuis le Code

Dans le setup() ou une automation :

```cpp
// Reset programmé
this->reset_safety_mode();
```

### Vérifier l'État

```cpp
// Vérifier si en mode sécurité
if (this->is_safety_triggered()) {
    ESP_LOGW(TAG, "Cover is in safety mode");
}
```

### Reset Conditionnel

```yaml
automation:
  - trigger:
      - platform: template
        value_template: "{{ states('sensor.gate_attempts') | int > 3 }}"
    condition:
      - condition: time
        after: "22:00:00"
        before: "06:00:00"
    action:
      - impulse_cover.reset_safety:
          id: my_gate
```

## 📊 Monitoring Recommandé

Créez ces capteurs pour surveiller l'activité :

```yaml
sensor:
  - platform: template
    name: "Portail Cycles Sécurité"
    lambda: |-
      return id(my_gate).get_safety_cycle_count();
    update_interval: 10s

binary_sensor:
  - platform: template
    name: "Portail Mode Sécurité"
    lambda: |-
      return id(my_gate).is_safety_triggered();
```

## 🎯 Bonnes Pratiques

1. **Prévention** : Évitez les commandes rapides répétées
2. **Surveillance** : Monitorez les logs régulièrement
3. **Maintenance** : Reset préventif pendant les maintenances
4. **Configuration** : Ajustez les seuils selon votre usage
5. **Documentation** : Documentez vos configurations spécifiques

---

*Ce guide vous aide à gérer efficacement le système de sécurité de votre Impulse Cover. Pour des questions spécifiques, consultez les logs ESPHome ou créez une issue sur GitHub.*
