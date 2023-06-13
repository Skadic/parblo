parblo
====================

`parblo` provides block trees constructed in parallel (soon). This implementation centers around the :cpp:class:`BlockTree <parblo::BlockTree>` class.

Use the :cpp:func:`constructor <parblo::BlockTree::BlockTree>` and provide the construction algorithm.
At this point, only a :cpp:class:`sequential algorithm<parblo::Sequential>` is WIP.

An example might look like this::

    #include <parblo/block_tree/block_tree.hpp>
    #include <parblo/block_tree/construction/sequential.hpp>

    std::string s = ...;
    parblo::BlockTree bt(s, 4, 8, parblo::Sequential());


.. toctree::
   :maxdepth: 2

   self
   api/library_root