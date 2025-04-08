N = 10  # Plus grand => lissage plus fort

# Contexte du filtre
buffer = [0.0] * N
index = 0
count = 0
sum_values = 0.0

def calculate_moving_average(new_sample):
    global buffer, index, count, sum_values

    sum_values -= buffer[index]
    buffer[index] = new_sample
    sum_values += new_sample

    index = (index + 1) % N
    if count < N:
        count += 1

    return sum_values / count

# Données simulées
array_data = [
     33, 33, 33, 33, 10, 33, 33, 33,
     45, 45, 45, 45, 45, 45, 45, 45,
     45, 45, 45, 45, 45, 45, 45, 45,
     60, 60, 60, 20, 60, 60, 60, 60,
     60, 60, 60, 60, 60, 60, 60, 60,
     10, 10, 10, 10, 10, 10, 10, 10,
     10, 10, 10, 10, 10, 10, 10, 10,
     33, 33, 33, 33, 33, 33, 33, 33,
     33, 33, 33, 5,  33, 33, 33, 33,
     45, 45, 70, 45, 45, 45, 45, 45,
     45, 45, 45, 45, 45, 45, 45, 45,
     60, 60, 60, 60, 60, 60, 15, 60,
     60, 60, 60, 60, 60, 60, 60, 60,
     10, 10, 10, 10, 10, 10, 10, 10,
     10, 10, 10, 10, 10, 10, 10, 10,
]

# Simulation de boucle infinie comme dans un microcontrôleur
for i, val in enumerate(array_data):
    moyenne = calculate_moving_average(val)
    print(f"Moyenne = {moyenne:.2f}")
