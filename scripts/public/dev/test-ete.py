import os

location = os.environ['HOME'] + '/test/Library/Application Support/Google/Chrome/External Extensions'
if os.path.exists(location):
    print("Not exists")

quit()

os.makedirs(location)
ete = '{\n\t"external_update_url": "https://clients2.google.com/service/update2/crx"\n}'

f = open(location+"/kpmiofpmlciacjblommkcinncmneeoaa.json", 'w')
f.write(ete)  # python will convert \n to os.linesep
f.close()
