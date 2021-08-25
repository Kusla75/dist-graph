import os

# Absolute path of project repo
path = os.path.dirname(__file__)
path = os.path.split(path)[0]

# Dataset paths
karate_path = path + '\\datasets\\soc-karate.mtx'
fb_pages_path = path + '\\datasets\\fb-pages-government.edges'
pokec_path = path + '\\datasets\\soc-pokec-relationships.txt'

solutions_path = path + '\\solutions\\'