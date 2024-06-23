import requests
from requests.auth import HTTPBasicAuth
import filecmp
import os

def printResponse(r):
	print("==STATUSCODE==")
	print(r.status_code)
	print("==HEADERS==")
	print(r.headers)
	print("==BODY==")
	print(r.text)
	print("******************\n")


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


# upload file
def upload_file(url, file_path):
    """Uploads a file to the specified URL."""
    # Use 'with' to ensure the file is closed after being uploaded
    with open(file_path, 'rb') as file:
        files = {'file': file}
        response = requests.post(url, files=files)
    return response

def compare_files(file1, file2):
    """Compares two files to see if they are identical."""
    return filecmp.cmp(file1, file2, shallow=False)

# Define the URL and file paths
upload_url = 'http://localhost:4243/upload'
original_file_path = '5mb.txt'
uploaded_file_path = 'www/files/5mb.txt'  # Adjusted to match the uploaded file name

# Step 1: Upload the file
response = upload_file(upload_url, original_file_path)
if response.status_code == 200:
    print("File uploaded successfully.")
else:
    print(f"Failed to upload file. Status code: {response.status_code}")

# Step 2: Compare the original file with the uploaded file
# Ensure the server has had time to process and save the file before comparison
if compare_files(original_file_path, uploaded_file_path):
    print("The uploaded file matches the original file.")
else:
    print("The uploaded file does not match the original file.")