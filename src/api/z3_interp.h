/*++
Copyright (c) 2014 Microsoft Corporation

Module Name:

    z3_interp.h

Abstract:

    API for interpolation

Author:

    Kenneth McMillan (kenmcmil)

Notes:

--*/
#ifndef _Z3_INTERPOLATION_H_
#define _Z3_INTERPOLATION_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /**
    \defgroup capi C API

    */

    /*@{*/

    /**
        @name Interpolation API
        */
    /*@{*/

    /**
    \brief \mlh mk_interp c a \endmlh
    Create an AST node marking a formula position for interpolation.

    The node \c a must have Boolean sort.

    def_API('Z3_mk_interpolant', AST, (_in(CONTEXT), _in(AST)))
    */
    Z3_ast Z3_API Z3_mk_interpolant(__in Z3_context c, __in Z3_ast a);


    /** \brief This function generates a Z3 context suitable for generation of
    interpolants. Formulas can be generated as abstract syntax trees in
    this context using the Z3 C API.

    Interpolants are also generated as AST's in this context.

    If cfg is non-null, it will be used as the base configuration
    for the Z3 context. This makes it possible to set Z3 options
    to be used during interpolation. This feature should be used
    with some caution however, as it may be that certain Z3 options
    are incompatible with interpolation.

    def_API('Z3_mk_interpolation_context', CONTEXT, (_in(CONFIG),))

    */

    Z3_context Z3_API Z3_mk_interpolation_context(__in Z3_config cfg);

    /** Compute an interpolant from a refutation. This takes a proof of
        "false" from a set of formulas C, and an interpolation
        pattern. The pattern pat is a formula combining the formulas in C
        using logical conjunction and the "interp" operator (see
        #Z3_mk_interpolant). This interp operator is logically the identity
        operator. It marks the sub-formulas of the pattern for which interpolants should
        be computed. The interpolant is a map sigma from marked subformulas to
        formulas, such that, for each marked subformula phi of pat (where phi sigma
        is phi with sigma(psi) substituted for each subformula psi of phi such that
        psi in dom(sigma)):

        1) phi sigma implies sigma(phi), and

        2) sigma(phi) is in the common uninterpreted vocabulary between
        the formulas of C occurring in phi and those not occurring in
        phi

        and moreover pat sigma implies false. In the simplest case
        an interpolant for the pattern "(and (interp A) B)" maps A
        to an interpolant for A /\ B.

        The return value is a vector of formulas representing sigma. The
        vector contains sigma(phi) for each marked subformula of pat, in
        pre-order traversal. This means that subformulas of phi occur before phi
        in the vector. Also, subformulas that occur multiply in pat will
        occur multiply in the result vector.

        In particular, calling Z3_get_interpolant on a pattern of the
        form (interp ... (interp (and (interp A_1) A_2)) ... A_N) will
        result in a sequence interpolant for A_1, A_2,... A_N.

        Neglecting interp markers, the pattern must be a conjunction of
        formulas in C, the set of premises of the proof. Otherwise an
        error is flagged.

        Any premises of the proof not present in the pattern are
        treated as "background theory". Predicate and function symbols
        occurring in the background theory are treated as interpreted and
        thus always allowed in the interpolant.

        Interpolant may not necessarily be computable from all
        proofs. To be sure an interpolant can be computed, the proof
        must be generated by an SMT solver for which interpoaltion is
        supported, and the premises must be expressed using only
        theories and operators for which interpolation is supported.

        Currently, the only SMT solver that is supported is the legacy
        SMT solver. Such a solver is available as the default solver in
        #Z3_context objects produced by #Z3_mk_interpolation_context.
        Currently, the theories supported are equality with
        uninterpreted functions, linear integer arithmetic, and the
        theory of arrays (in SMT-LIB terms, this is AUFLIA).
        Quantifiers are allowed. Use of any other operators (including
        "labels") may result in failure to compute an interpolant from a
        proof.

        Parameters:

        \param c logical context.
        \param pf a refutation from premises (assertions) C
        \param pat an interpolation pattern over C
        \param p parameters

        def_API('Z3_get_interpolant', AST_VECTOR, (_in(CONTEXT), _in(AST), _in(AST), _in(PARAMS)))
        */

    Z3_ast_vector Z3_API Z3_get_interpolant(__in Z3_context c, __in Z3_ast pf, __in Z3_ast pat, __in Z3_params p);

    /* Compute an interpolant for an unsatisfiable conjunction of formulas.

       This takes as an argument an interpolation pattern as in
       #Z3_get_interpolant. This is a conjunction, some subformulas of
       which are marked with the "interp" operator (see #Z3_mk_interpolant).

       The conjunction is first checked for unsatisfiability. The result
       of this check is returned in the out parameter "status". If the result
       is unsat, an interpolant is computed from the refutation as in #Z3_get_interpolant
       and returned as a vector of formulas. Otherwise the return value is
       an empty formula.

       See #Z3_get_interpolant for a discussion of supported theories.

       The advantage of this function over #Z3_get_interpolant is that
       it is not necessary to create a suitable SMT solver and generate
       a proof. The disadvantage is that it is not possible to use the
       solver incrementally.

       Parameters:

       \param c logical context.
       \param pat an interpolation pattern
       \param p parameters for solver creation
       \param status returns the status of the sat check
       \param model returns model if satisfiable

       Return value: status of SAT check

       def_API('Z3_compute_interpolant', INT, (_in(CONTEXT), _in(AST), _in(PARAMS), _out(AST_VECTOR), _out(MODEL)))
       */

    Z3_lbool Z3_API Z3_compute_interpolant(__in Z3_context c,
                                           __in Z3_ast pat,
                                           __in Z3_params p,
                                           __out Z3_ast_vector *interp,
                                           __out Z3_model *model);

    /** Return a string summarizing cumulative time used for
        interpolation.  This string is purely for entertainment purposes
        and has no semantics.

        \param ctx The context (currently ignored)


        def_API('Z3_interpolation_profile', STRING, (_in(CONTEXT),))
        */

    Z3_string Z3_API Z3_interpolation_profile(__in Z3_context ctx);

    /**
       \brief Read an interpolation problem from file.

       \param ctx The Z3 context. This resets the error handler of ctx.
       \param filename The file name to read.
       \param num Returns length of sequence.
       \param cnsts Returns sequence of formulas (do not free)
       \param parents Returns the parents vector (or NULL for sequence)
       \param error Returns an error message in case of failure (do not free the string)
       \param num_theory Number of theory terms
       \param theory Theory terms

       Returns true on success.

       File formats: Currently two formats are supported, based on
       SMT-LIB2. For sequence interpolants, the sequence of constraints is
       represented by the sequence of "assert" commands in the file.

       For tree interpolants, one symbol of type bool is associated to
       each vertex of the tree. For each vertex v there is an "assert"
       of the form:

       (implies (and c1 ... cn f) v)

       where c1 .. cn are the children of v (which must precede v in the file)
       and f is the formula assiciated to node v. The last formula in the
       file is the root vertex, and is represented by the predicate "false".

       A solution to a tree interpolation problem can be thought of as a
       valuation of the vertices that makes all the implications true
       where each value is represented using the common symbols between
       the formulas in the subtree and the remainder of the formulas.

       def_API('Z3_read_interpolation_problem', INT, (_in(CONTEXT), _out(UINT), _out_managed_array(1, AST), _out_managed_array(1, UINT), _in(STRING), _out(STRING), _out(UINT), _out_managed_array(6, AST)))

       */

    int Z3_API Z3_read_interpolation_problem(__in Z3_context ctx,
                                             __out unsigned *num,
                                             __out Z3_ast *cnsts[],
                                             __out unsigned *parents[],
                                             __in Z3_string filename,
                                             __out_opt Z3_string_ptr error,
                                             __out unsigned *num_theory,
                                             __out Z3_ast *theory[]);



    /** Check the correctness of an interpolant. The Z3 context must
        have no constraints asserted when this call is made. That means
        that after interpolating, you must first fully pop the Z3
        context before calling this. See Z3_interpolate for meaning of parameters.

        \param ctx The Z3 context. Must be generated by Z3_mk_interpolation_context
        \param num The number of constraints in the sequence
        \param cnsts Array of constraints (AST's in context ctx)
        \param parents The parents vector (or NULL for sequence)
        \param interps The interpolant to check
        \param error Returns an error message if interpolant incorrect (do not free the string)
        \param num_theory Number of theory terms
        \param theory Theory terms

        Return value is Z3_L_TRUE if interpolant is verified, Z3_L_FALSE if
        incorrect, and Z3_L_UNDEF if unknown.

        def_API('Z3_check_interpolant', INT, (_in(CONTEXT), _in(UINT), _in_array(1, AST), _in_array(1, UINT), _in_array(1, AST), _out(STRING), _in(UINT), _in_array(6, AST)))
        */

    int Z3_API Z3_check_interpolant(__in Z3_context ctx,
                                    __in unsigned num,
                                    __in_ecount(num) Z3_ast cnsts[],
                                    __in_ecount(num) unsigned parents[],
                                    __in_ecount(num - 1) Z3_ast *interps,
                                    __out_opt Z3_string_ptr error,
                                    __in unsigned num_theory,
                                    __in_ecount(num_theory) Z3_ast theory[]);

    /** Write an interpolation problem to file suitable for reading with
        Z3_read_interpolation_problem. The output file is a sequence
        of SMT-LIB2 format commands, suitable for reading with command-line Z3
        or other interpolating solvers.

        \param ctx The Z3 context. Must be generated by z3_mk_interpolation_context
        \param num The number of constraints in the sequence
        \param cnsts Array of constraints
        \param parents The parents vector (or NULL for sequence)
        \param filename The file name to write
        \param num_theory Number of theory terms
        \param theory Theory terms

        def_API('Z3_write_interpolation_problem', VOID, (_in(CONTEXT), _in(UINT), _in_array(1, AST), _in_array(1, UINT), _in(STRING), _in(UINT), _in_array(5, AST)))
        */

    void Z3_API  Z3_write_interpolation_problem(__in Z3_context ctx,
                                                __in unsigned num,
                                                __in_ecount(num) Z3_ast cnsts[],
                                                __in_ecount(num) unsigned parents[],
                                                __in Z3_string filename,
                                                __in unsigned num_theory,
                                                __in_ecount(num_theory) Z3_ast theory[]);

    /*@}*/
    /*@}*/

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
