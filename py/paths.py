import os

# Absolute path of project repo
path = os.path.dirname(__file__)
path = os.path.split(path)[0]

# Dataset paths
soc_karate_path = path + '\\datasets\\soc-karate.mtx'
fb_pages_path = path + '\\datasets\\fb-pages-government.edges'
fb_pages_food_path = path + '\\datasets\\fb-pages-food.edges'
socfb_caltech_path = path + '\\datasets\\socfb-caltech.mtx'
socfb_mich = path + '\\datasets\\socfb-Mich67.mtx'
soc_hamsterster_path = path + '\\datasets\\soc-hamsterster.edges'
pokec_path = path + '\\datasets\\soc-pokec-relationships.txt'

solutions_path = path + '\\solutions\\'