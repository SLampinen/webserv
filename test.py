import requests
from requests.auth import HTTPBasicAuth

def printResponse(r):
	print("==STATUSCODE==")
	print(r.status_code)
	print("==HEADERS==")
	print(r.headers)
	print("==BODY==")
	print(r.text)
	print("******************\n")

for _ in range(20000):
    print("GET on 4243")
    r = requests.get('http://localhost:4243')
    printResponse(r)

    print("GET on 4243 with no language parameter")
    r = requests.get('http://localhost:4243')
    printResponse(r)

    print("GET on 4243with preferred language FR")
    headers = {'Accept-Language': 'fr'}
    r = requests.get('http://localhost:4243', headers=headers)
    printResponse(r)

    print("GET on 4243with preferred language EN")
    headers = {'Accept-Language': 'en'}
    r = requests.get('http://localhost:4243', headers=headers)
    printResponse(r)

    print("GET on 4243 with preferred language FR and preferred encoding utf8")
    headers = {'Accept-Language': 'fr', 'Accept-Charset': 'utf8'}
    r = requests.get('http://localhost:4243', headers=headers)
    printResponse(r)

    print("GET on 4243 with preferred language EN and preferred encoding utf8")
    headers = {'Accept-Language': 'en', 'Accept-Charset': 'utf8'}
    r = requests.get('http://localhost:4243', headers=headers)
    printResponse(r)

    print("GET on 4243/upload")
    r = requests.get('http://localhost:4243/upload')
    printResponse(r)
