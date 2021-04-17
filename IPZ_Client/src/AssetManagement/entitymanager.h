#pragma once
// unordered_map<int, weak?_pointer>
// large preallocated memory block
// deleted entities marked as free memory
// free blocks of memory in std::set sorted by size
// we would have to check on insertion and deletion if we need to unfragment something
// or we can just reallocate everything when the deletions reach certain treshold
// lookup first element to see if we can allocate if not allocate at the end
// additional unordered_map<int, vector<ptr>> to quickly get entities by type?
// get weak_pointer when we retrieve entities?
// this might get complicated and not be very fast
class EntityManager
{
public:
    EntityManager();
};

