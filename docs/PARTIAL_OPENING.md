# Configuration des Temps et Ouverture Partielle

## Paramètres de Configuration

Votre composant impulse cover supporte maintenant la configuration complète des temps d'ouverture et de fermeture pour un calcul précis des pourcentages d'ouverture et la possibilité d'ouverture partielle.

### Paramètres Obligatoires

```yaml
cover:
  - platform: impulse_cover
    name: "Mon Portail"
    id: mon_portail
    output: relay_output
    
    # OBLIGATOIRE: Temps pour une ouverture complète (0% → 100%)
    open_duration: 20s
    
    # OBLIGATOIRE: Temps pour une fermeture complète (100% → 0%) 
    close_duration: 18s
```

### Paramètres Optionnels

```yaml
cover:
  - platform: impulse_cover
    # ... paramètres obligatoires ...
    
    # Délai entre les impulsions de contrôle (défaut: 500ms)
    pulse_delay: 800ms
    
    # Timeout de sécurité (défaut: 60s)
    safety_timeout: 90s
    
    # Nombre max de cycles avant arrêt sécurité (défaut: 5)
    safety_max_cycles: 3
    
    # Capteurs de fin de course (optionnels)
    open_sensor: capteur_ouverture
    close_sensor: capteur_fermeture
    open_sensor_inverted: true   # Capteur actif LOW
    close_sensor_inverted: true  # Capteur actif LOW
```

## Utilisation de l'Ouverture Partielle

### Via Home Assistant

1. **Interface graphique** : Utilisez le slider de position sur la carte cover
2. **Service call** :
```yaml
service: cover.set_cover_position
target:
  entity_id: cover.mon_portail
data:
  position: 30  # 30% ouvert
```

### Via ESPHome Web Server

Accédez à `http://IP_DEVICE` et utilisez l'interface web pour contrôler la position.

### Exemples de Positions Utiles

```yaml
# Ouverture piéton (30%)
- position: 0.3

# Ouverture petits véhicules (60%) 
- position: 0.6

# Ouverture complète véhicules lourds (100%)
- position: 1.0

# Fermeture complète
- position: 0.0
```

## Calcul de Position

Le composant calcule automatiquement la position en temps réel :

1. **Position de départ** : Mémorisée au début de chaque mouvement
2. **Progression** : Calculée en fonction du temps écoulé et de la durée configurée
3. **Position actuelle** : `position_départ ± (distance × progression)`

### Exemple de Calcul

Si votre portail est à 20% d'ouverture et vous demandez 70% :
- Distance à parcourir : `70% - 20% = 50%`
- Temps nécessaire : `20s × 0.5 = 10s` (pour open_duration=20s)
- Position mise à jour en temps réel pendant 10 secondes

## Configuration avec Capteurs de Fin de Course

Les capteurs permettent une position précise :

```yaml
binary_sensor:
  - platform: gpio
    pin: 
      number: GPIO4
      mode: INPUT_PULLUP
    name: "Capteur Ouverture"
    id: capteur_ouverture
    
  - platform: gpio
    pin:
      number: GPIO5
      mode: INPUT_PULLUP  
    name: "Capteur Fermeture"
    id: capteur_fermeture

cover:
  - platform: impulse_cover
    name: "Portail Précis"
    id: portail_precis
    # ... autres paramètres ...
    
    # Capteurs pour correction automatique
    open_sensor: capteur_ouverture
    close_sensor: capteur_fermeture
    open_sensor_inverted: true   # Pullup = actif LOW
    close_sensor_inverted: true  # Pullup = actif LOW
```

### Fonctionnement avec Capteurs

1. **Auto-correction** : Si un capteur se déclenche, la position est automatiquement corrigée (0% ou 100%)
2. **Arrêt précis** : Le mouvement s'arrête immédiatement au déclenchement
3. **Sécurité renforcée** : Les capteurs empêchent la surcharge du moteur

## Ajustement des Temps

### Comment Mesurer les Temps

1. **Chronomètre** : Mesurez manuellement le temps d'ouverture/fermeture complète
2. **Test progressif** : Commencez avec des valeurs approximatives puis ajustez
3. **Vérification** : Testez plusieurs cycles complets pour confirmer la précision

### Exemple d'Ajustement

```yaml
# Première estimation
open_duration: 25s
close_duration: 25s

# Après test : ouverture plus rapide
open_duration: 20s   # Ajusté après mesure
close_duration: 25s  # Correct

# Test final avec ouverture partielle
# Demander 50% et vérifier si c'est vraiment à mi-course
```

## Dépannage

### Position Imprécise
- Vérifiez que `open_duration` et `close_duration` correspondent à la réalité
- Testez plusieurs cycles complets pour validation
- Utilisez des capteurs de fin de course pour auto-correction

### Mouvement Trop Lent/Rapide
- Ajustez `pulse_delay` (plus court = plus réactif)
- Vérifiez que le moteur répond bien aux impulsions

### Arrêts Intempestifs
- Augmentez `safety_timeout` si nécessaire
- Réduisez `safety_max_cycles` si le portail a des problèmes mécaniques

## Configuration Exemple Complète

Voir `examples/partial-test.yaml` pour un exemple complet avec :
- Temps d'ouverture/fermeture configurés
- Capteurs de fin de course
- Interface web pour tests
- Paramètres de sécurité ajustés
