#!/usr/bin/env python3
"""
TPC-H Dataset Generator for NanoDB
Generates sample data for customer, orders, and lineitem tables
"""

import os
import random
import argparse
from pathlib import Path

# Column definitions for TPC-H tables
CUSTOMER_COLUMNS = [
    'c_custkey',    # INT
    'c_name',       # VARCHAR
    'c_address',    # VARCHAR
    'c_nationkey',  # INT
    'c_acctbal',    # DECIMAL
    'c_mktsegment', # VARCHAR
    'c_comment'     # VARCHAR
]

ORDERS_COLUMNS = [
    'o_orderkey',      # INT
    'o_custkey',       # INT
    'o_orderstatus',   # CHAR
    'o_totalprice',    # DECIMAL
    'o_orderdate',     # DATE
    'o_orderpriority', # CHAR
    'o_clerk',         # VARCHAR
    'o_shippriority',  # INT
    'o_comment'        # VARCHAR
]

LINEITEM_COLUMNS = [
    'l_orderkey',     # INT
    'l_partkey',      # INT
    'l_suppkey',      # INT
    'l_linenumber',   # INT
    'l_quantity',     # DECIMAL
    'l_extendedprice',# DECIMAL
    'l_discount',     # DECIMAL
    'l_tax',          # DECIMAL
    'l_returnflag',   # CHAR
    'l_linestatus',   # CHAR
    'l_shipdate',     # DATE
    'l_commitdate',   # DATE
    'l_receiptdate',  # DATE
    'l_shipinstruct', # VARCHAR
    'l_shipmode',     # VARCHAR
    'l_comment'       # VARCHAR
]

# Sample data pools
CUSTOMER_NAMES = ["Customer_" + str(i) for i in range(1, 1000)]
MKTSEGMENTS = ["AUTOMOBILE", "BUILDING", "FURNITURE", "HOUSEHOLD", "MACHINERY"]
COUNTRIES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30]
ORDERSTATUS = ["O", "P", "D", "F"]
PRIORITIES = ["1-URGENT", "2-HIGH", "3-MEDIUM", "4-NOT SPECIFIED", "5-LOW"]
SHIP_MODES = ["AIR", "REG AIR", "MAIL", "RAIL", "SHIP", "TRUCK", "FOB"]

def generate_customer(num_records=20000):
    """Generate customer table data"""
    print(f"Generating {num_records} customer records...")
    with open('datasets/customer.tbl', 'w') as f:
        for i in range(1, num_records + 1):
            custkey = i
            name = random.choice(CUSTOMER_NAMES)
            address = f"Address_{i}"
            nationkey = random.choice(COUNTRIES)
            acctbal = round(random.uniform(0, 10000), 2)
            mktsegment = random.choice(MKTSEGMENTS)
            comment = "Sample comment"
            
            line = f"{custkey}|{name}|{address}|{nationkey}|{acctbal}|{mktsegment}|{comment}\n"
            f.write(line)
    print(f"✓ Generated {num_records} customer records")

def generate_orders(num_records=30000, max_custkey=20000):
    """Generate orders table data"""
    print(f"Generating {num_records} order records...")
    with open('datasets/orders.tbl', 'w') as f:
        for i in range(1, num_records + 1):
            orderkey = i
            custkey = random.randint(1, max_custkey)
            orderstatus = random.choice(ORDERSTATUS)
            totalprice = round(random.uniform(100, 500000), 2)
            orderdate = f"1995-{random.randint(1,12):02d}-{random.randint(1,28):02d}"
            orderpriority = random.choice(PRIORITIES)
            clerk = f"Clerk_{random.randint(1, 100):03d}"
            shippriority = random.randint(0, 2)
            comment = "Order comment"
            
            line = f"{orderkey}|{custkey}|{orderstatus}|{totalprice}|{orderdate}|{orderpriority}|{clerk}|{shippriority}|{comment}\n"
            f.write(line)
    print(f"✓ Generated {num_records} order records")

def generate_lineitem(num_records=50000, max_orderkey=30000):
    """Generate lineitem table data"""
    print(f"Generating {num_records} lineitem records...")
    with open('datasets/lineitem.tbl', 'w') as f:
        line_num = 1
        for i in range(1, num_records + 1):
            orderkey = random.randint(1, max_orderkey)
            partkey = random.randint(1, 100000)
            suppkey = random.randint(1, 10000)
            linenumber = random.randint(1, 7)
            quantity = random.randint(1, 50)
            extendedprice = round(random.uniform(100, 100000), 2)
            discount = round(random.uniform(0, 0.1), 3)
            tax = round(random.uniform(0, 0.08), 3)
            returnflag = random.choice(["R", "A", "N"])
            linestatus = random.choice(["O", "F"])
            shipdate = f"1995-{random.randint(1,12):02d}-{random.randint(1,28):02d}"
            commitdate = f"1995-{random.randint(1,12):02d}-{random.randint(1,28):02d}"
            receiptdate = f"1995-{random.randint(1,12):02d}-{random.randint(1,28):02d}"
            shipinstruct = "DELIVER IN PERSON"
            shipmode = random.choice(SHIP_MODES)
            comment = "Lineitem comment"
            
            line = f"{orderkey}|{partkey}|{suppkey}|{linenumber}|{quantity}|{extendedprice}|{discount}|{tax}|{returnflag}|{linestatus}|{shipdate}|{commitdate}|{receiptdate}|{shipinstruct}|{shipmode}|{comment}\n"
            f.write(line)
    print(f"✓ Generated {num_records} lineitem records")

def main():
    parser = argparse.ArgumentParser(description='Generate TPC-H datasets for NanoDB')
    parser.add_argument('--customer', type=int, default=20000, help='Number of customer records')
    parser.add_argument('--orders', type=int, default=30000, help='Number of order records')
    parser.add_argument('--lineitem', type=int, default=50000, help='Number of lineitem records')
    
    args = parser.parse_args()
    
    # Create datasets directory
    os.makedirs('datasets', exist_ok=True)
    
    print("\n" + "="*60)
    print("  TPC-H Dataset Generator for NanoDB")
    print("="*60 + "\n")
    
    try:
        generate_customer(args.customer)
        generate_orders(args.orders, args.customer)
        generate_lineitem(args.lineitem, args.orders)
        
        print("\n" + "="*60)
        print("  Dataset Generation Complete!")
        print("="*60)
        print(f"\nGenerated files in datasets/:")
        print(f"  ✓ customer.tbl     ({args.customer:,} records)")
        print(f"  ✓ orders.tbl       ({args.orders:,} records)")
        print(f"  ✓ lineitem.tbl     ({args.lineitem:,} records)")
        print(f"\nTotal records: {args.customer + args.orders + args.lineitem:,}\n")
        
    except Exception as e:
        print(f"\nError: {e}")
        return 1
    
    return 0

if __name__ == '__main__':
    exit(main())
