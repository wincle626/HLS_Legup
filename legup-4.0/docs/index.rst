.. LegUp documentation master file, created by
   sphinx-quickstart on Sat Nov  5 14:49:27 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. Welcome to LegUp's documentation!
.. =================================

LegUp |version| Documentation
==============================

LegUp is a high-level synthesis research infrastructure being actively developed at the
University of Toronto since early 2010. 
Our goal is to allow researchers to experiment with new high-level synthesis
algorithms without building a new infrastructure from scratch.
Our long-term vision is to make FPGA programming easier for software developers.

.. LegUp includes:
 
  * C to Verilog high-level synthesis tool. Tested on Linux 32/64-bit.
  * CHStone benchmark suite and dhrystone benchmarks
  * Tiger MIPS processor from the University of Cambridge

.. An important objective was to have a way of measuring the quality of results
   of new algorithms. For this reason we designed LegUp to support the CHStone
   benchmark suite, which are 12 C benchmarks that are larger than the typical
   benchmarks studied in prior work.





The documentation is comprised of the following sections:

    * :ref:`getstarted`: Installation and a quick start guide
    * :ref:`userguide`: How to use LegUp to generate hardware
    * :ref:`hwarch`: Details of the synthesized circuit architecture
    * :ref:`progman`: Describes the layout of the LegUp codebase and the class hierarchy
    * :ref:`constraints`: Constraints manual
    * :ref:`FAQ`: Frequently asked questions
    * :ref:`release`: New features and known problems with each release

If you have questions, patches, and suggestions please email them to the LegUp
development mailing list, legup-dev@legup.org, or email us directly at legup@eecg.toronto.edu.

If you find a bug in LegUp, please file it in `Bugzilla <http://legup.org/bugs/>`_.

.. toctree::
   :maxdepth: 2

   gettingstarted
   userguide
   hwarchitecture
   programmermanual
   debug
   hybridpartition
   pcie
   xilinx
   faq
   constraintsmanual
   releasenotes

.. Indices and tables
.. ==================
.. 
.. * :ref:`genindex`
.. * :ref:`modindex` 
.. * :ref:`search`

