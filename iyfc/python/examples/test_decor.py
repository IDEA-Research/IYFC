from utils import hfc_math_test, hfc_query_test
import pyiyfc as iyfc


# Test mathematical calculations, PSI follows the same pattern
@hfc_math_test('./encrypt')
def cal(a, b, c):
    return (a + b) * c, a * (b - c)

# Test query operations
@hfc_query_test('./encrypt')
def query(a, b, c, d):
    return (a <= b) * (c != d)

# In the encryption test, random inputs will be generated, and here invalid inputs are used for plain-text calculations
print(cal(4, 5, 9))
print(query(1, 2, 3, 4))



